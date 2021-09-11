/*
 *     Copyright (C) 2019  Filippo Scognamiglio
 *
 *     This program is free software: you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation, either version 3 of the License, or
 *     (at your option) any later version.
 *
 *     This program is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU General Public License for more details.
 *
 *     You should have received a copy of the GNU General Public License
 *     along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

package com.swordfish.libretrodroid

import android.app.ActivityManager
import android.content.Context
import android.opengl.GLSurfaceView
import android.util.Log
import android.view.InputDevice
import android.view.KeyEvent
import android.view.MotionEvent
import android.view.WindowManager
import androidx.lifecycle.Lifecycle
import androidx.lifecycle.LifecycleObserver
import androidx.lifecycle.LifecycleOwner
import androidx.lifecycle.OnLifecycleEvent
import com.jakewharton.rxrelay2.BehaviorRelay
import com.jakewharton.rxrelay2.PublishRelay
import com.swordfish.libretrodroid.gamepad.GamepadsManager
import io.reactivex.Observable
import java.util.*
import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.opengles.GL10
import kotlin.properties.Delegates

class GLRetroView(
        context: Context,
        private val data: GLRetroViewData
) : AspectRatioGLSurfaceView(context), LifecycleObserver {

    var audioEnabled: Boolean by Delegates.observable(true) { _, _, value ->
        LibretroDroid.setAudioEnabled(value)
    }

    var frameSpeed: Int by Delegates.observable(1) { _, _, value ->
        LibretroDroid.setFrameSpeed(value)
    }

    private val openGLESVersion: Int

    private var isGameLoaded = false
    private var isEmulationReady = false
    private var isAborted = false

    private val retroGLEventsSubject = BehaviorRelay.create<GLRetroEvents>()
    private val retroGLIssuesErrors = PublishRelay.create<Int>()

    private val rumbleEventsSubject = BehaviorRelay.create<RumbleEvent>()

    private var lifecycle: Lifecycle? = null

    init {
        openGLESVersion = getGLESVersion(context)
        preserveEGLContextOnPause = true
        setEGLContextClientVersion(openGLESVersion)
        setRenderer(Renderer())
        keepScreenOn = true
    }

    @OnLifecycleEvent(Lifecycle.Event.ON_CREATE)
    fun onCreate(lifecycleOwner: LifecycleOwner) = catchExceptions {
        lifecycle = lifecycleOwner.lifecycle
        LibretroDroid.create(
            openGLESVersion,
            data.coreFilePath,
            data.systemDirectory,
            data.savesDirectory,
            data.variables,
            data.shader,
            getDefaultRefreshRate(),
            data.preferLowLatencyAudio,
            data.gameVirtualFiles.isNotEmpty(),
            getDeviceLanguage()
        )
        LibretroDroid.setRumbleEnabled(data.rumbleEventsEnabled)
    }

    @OnLifecycleEvent(Lifecycle.Event.ON_DESTROY)
    fun onDestroy() = catchExceptions {
        LibretroDroid.destroy()
        lifecycle = null
    }

    private fun getDeviceLanguage() = Locale.getDefault().language

    private fun getDefaultRefreshRate(): Float {
        return (context.getSystemService(Context.WINDOW_SERVICE) as WindowManager).defaultDisplay.refreshRate
    }

    fun sendKeyEvent(action: Int, keyCode: Int, port: Int = 0) {
        queueEvent { LibretroDroid.onKeyEvent(port, action, keyCode) }
    }

    fun sendMotionEvent(source: Int, xAxis: Float, yAxis: Float, port: Int = 0) {
        queueEvent { LibretroDroid.onMotionEvent(port, source, xAxis, yAxis) }
    }

    override fun onTouchEvent(event: MotionEvent?): Boolean {
        when (event?.actionMasked) {
            MotionEvent.ACTION_DOWN -> {
                sendTouchEvent(event)
            }
            MotionEvent.ACTION_MOVE -> {
                sendTouchEvent(event)
            }
            MotionEvent.ACTION_UP -> {
                sendMotionEvent(MOTION_SOURCE_POINTER, -1f, -1f)
            }
        }
        return true
    }

    private fun clamp(x: Float, min: Float, max: Float) = minOf(maxOf(x, min), max)

    private fun sendTouchEvent(event: MotionEvent) {
        val x = clamp(event.x / width, 0f, 1f)
        val y = clamp(event.y / height, 0f, 1f)
        sendMotionEvent(MOTION_SOURCE_POINTER, x, y)
    }

    fun serializeState(): ByteArray {
        return LibretroDroid.serializeState()
    }

    fun unserializeState(data: ByteArray): Boolean {
        return LibretroDroid.unserializeState(data)
    }

    fun serializeSRAM(): ByteArray {
        return LibretroDroid.serializeSRAM()
    }

    fun unserializeSRAM(data: ByteArray): Boolean {
        return LibretroDroid.unserializeSRAM(data)
    }

    fun reset() {
        LibretroDroid.reset()
    }

    fun getGLRetroEvents(): Observable<GLRetroEvents> {
        return retroGLEventsSubject
    }

    fun getGLRetroErrors(): Observable<Int> {
        return retroGLIssuesErrors
    }

    fun getRumbleEvents(): Observable<RumbleEvent> {
        return rumbleEventsSubject
    }

    fun getControllers(): Array<Array<Controller>> {
        return LibretroDroid.getControllers()
    }

    fun setControllerType(port: Int, type: Int) {
        LibretroDroid.setControllerType(port, type)
    }

    fun getVariables(): Array<Variable> {
        return LibretroDroid.getVariables()
    }

    fun updateVariables(vararg variables: Variable) {
        variables.forEach {
            LibretroDroid.updateVariable(it)
        }
    }

    fun getAvailableDisks() = LibretroDroid.availableDisks()
    fun getCurrentDisk() = LibretroDroid.currentDisk()
    fun changeDisk(index: Int) = LibretroDroid.changeDisk(index)

    private fun getGLESVersion(context: Context): Int {
        val activityManager = context.getSystemService(Context.ACTIVITY_SERVICE) as ActivityManager
        return if (activityManager.deviceConfigurationInfo.reqGlEsVersion >= 0x30000) { 3 } else { 2 }
    }

    override fun onKeyDown(keyCode: Int, event: KeyEvent?): Boolean {
        val mappedKey = GamepadsManager.getGamepadKeyEvent(keyCode)
        val port = (event?.device?.controllerNumber ?: 0) - 1

        if (event != null && port >= 0 && keyCode in GamepadsManager.GAMEPAD_KEYS) {
            sendKeyEvent(KeyEvent.ACTION_DOWN, mappedKey, port)
            return true
        }
        return super.onKeyDown(keyCode, event)
    }

    override fun onKeyUp(keyCode: Int, event: KeyEvent?): Boolean {
        val mappedKey = GamepadsManager.getGamepadKeyEvent(keyCode)
        val port = (event?.device?.controllerNumber ?: 0) - 1

        if (event != null && port >= 0 && keyCode in GamepadsManager.GAMEPAD_KEYS) {
            sendKeyEvent(KeyEvent.ACTION_UP, mappedKey, port)
            return true
        }
        return super.onKeyUp(keyCode, event)
    }

    override fun onGenericMotionEvent(event: MotionEvent?): Boolean {
        val port = (event?.device?.controllerNumber ?: 0) - 1
        if (port >= 0) {
            when (event?.source) {
                InputDevice.SOURCE_JOYSTICK -> {
                    sendMotionEvent(
                        MOTION_SOURCE_DPAD,
                        event.getAxisValue(MotionEvent.AXIS_HAT_X),
                        event.getAxisValue(MotionEvent.AXIS_HAT_Y),
                        port
                    )
                    sendMotionEvent(
                        MOTION_SOURCE_ANALOG_LEFT,
                        event.getAxisValue(MotionEvent.AXIS_X),
                        event.getAxisValue(MotionEvent.AXIS_Y),
                        port
                    )
                    sendMotionEvent(
                        MOTION_SOURCE_ANALOG_RIGHT,
                        event.getAxisValue(MotionEvent.AXIS_Z),
                        event.getAxisValue(MotionEvent.AXIS_RZ),
                        port
                    )
                }
            }
        }
        return super.onGenericMotionEvent(event)
    }

    // These functions are called only after the GLSurfaceView has been created.
    private inner class RenderLifecycleObserver : LifecycleObserver {
        @OnLifecycleEvent(Lifecycle.Event.ON_RESUME)
        private fun resume() = catchExceptions {
            LibretroDroid.resume()
            onResume()
            refreshAspectRatio()
            isEmulationReady = true
        }

        @OnLifecycleEvent(Lifecycle.Event.ON_PAUSE)
        private fun pause() = catchExceptions {
            isEmulationReady = false
            onPause()
            LibretroDroid.pause()
        }
    }

    inner class Renderer : GLSurfaceView.Renderer {
        override fun onDrawFrame(gl: GL10) = catchExceptions {
            if (isEmulationReady) {
                LibretroDroid.step(this@GLRetroView)
                retroGLEventsSubject.accept(GLRetroEvents.FrameRendered)
            }
        }

        override fun onSurfaceChanged(gl: GL10, width: Int, height: Int) = catchExceptions {
            Thread.currentThread().priority = Thread.MAX_PRIORITY
            LibretroDroid.onSurfaceChanged(width, height)
        }


        override fun onSurfaceCreated(gl: GL10, config: EGLConfig) = catchExceptions {
            Thread.currentThread().priority = Thread.MAX_PRIORITY
            initializeCore()
            retroGLEventsSubject.accept(GLRetroEvents.SurfaceCreated)
        }
    }

    private fun refreshAspectRatio() {
        val aspectRatio = LibretroDroid.getAspectRatio()
        KtUtils.runOnUIThread { setAspectRatio(aspectRatio) }
    }

    // These functions are called from the GL thread.
    private fun initializeCore() = catchExceptions {
        if (isGameLoaded) return@catchExceptions
        when {
            data.gameFilePath != null -> loadGameFromPath(data.gameFilePath!!)
            data.gameFileBytes != null -> loadGameFromBytes(data.gameFileBytes!!)
            data.gameVirtualFiles.isNotEmpty() -> loadGameFromVirtualFiles(data.gameVirtualFiles)
        }
        data.saveRAMState?.let {
            LibretroDroid.unserializeSRAM(data.saveRAMState)
            data.saveRAMState = null
        }
        LibretroDroid.onSurfaceCreated()
        isGameLoaded = true

        KtUtils.runOnUIThread {
            lifecycle?.addObserver(RenderLifecycleObserver())
        }
    }

    private fun loadGameFromVirtualFiles(virtualFiles: List<VirtualFile>) {
        val detachedVirtualFiles = virtualFiles
            .map { DetachedVirtualFile(it.virtualPath, it.fileDescriptor.detachFd()) }
        LibretroDroid.loadGameFromVirtualFiles(detachedVirtualFiles)
    }

    private fun loadGameFromBytes(gameFileBytes: ByteArray) {
        LibretroDroid.loadGameFromBytes(gameFileBytes)
    }

    private fun loadGameFromPath(gameFilePath: String) {
        LibretroDroid.loadGameFromPath(gameFilePath)
    }

    private fun catchExceptions(block: () -> Unit) {
        try {
            if (isAborted) return
            block()
        } catch (e: RetroException) {
            retroGLIssuesErrors.accept(e.errorCode)
            isAborted = true
        } catch (e: Exception) {
            Log.e(TAG_LOG, "Error in GLRetroView", e)
            retroGLIssuesErrors.accept(LibretroDroid.ERROR_GENERIC)
        }
    }

    /** This function gets called from the jni side.*/
    private fun sendRumbleEvent(port: Int, strengthWeak: Float, strengthStrong: Float) {
        rumbleEventsSubject.accept(RumbleEvent(port, strengthWeak, strengthStrong))
    }

    sealed class GLRetroEvents {
        object FrameRendered: GLRetroEvents()
        object SurfaceCreated: GLRetroEvents()
    }

    companion object {
        private val TAG_LOG = GLRetroView::class.java.simpleName

        const val MOTION_SOURCE_DPAD = LibretroDroid.MOTION_SOURCE_DPAD
        const val MOTION_SOURCE_ANALOG_LEFT = LibretroDroid.MOTION_SOURCE_ANALOG_LEFT
        const val MOTION_SOURCE_ANALOG_RIGHT = LibretroDroid.MOTION_SOURCE_ANALOG_RIGHT
        const val MOTION_SOURCE_POINTER = LibretroDroid.MOTION_SOURCE_POINTER

        const val SHADER_DEFAULT = LibretroDroid.SHADER_DEFAULT
        const val SHADER_CRT = LibretroDroid.SHADER_CRT
        const val SHADER_LCD = LibretroDroid.SHADER_LCD
        const val SHADER_SHARP = LibretroDroid.SHADER_SHARP

        const val ERROR_LOAD_LIBRARY = LibretroDroid.ERROR_LOAD_LIBRARY
        const val ERROR_LOAD_GAME = LibretroDroid.ERROR_LOAD_GAME
        const val ERROR_GL_NOT_COMPATIBLE = LibretroDroid.ERROR_GL_NOT_COMPATIBLE
        const val ERROR_SERIALIZATION = LibretroDroid.ERROR_SERIALIZATION
        const val ERROR_GENERIC = LibretroDroid.ERROR_GENERIC
    }
}
