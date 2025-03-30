/*
 *     Copyright (C) 2022  Filippo Scognamiglio
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
import android.graphics.PointF
import android.graphics.RectF
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
import androidx.lifecycle.coroutineScope
import com.swordfish.libretrodroid.KtUtils.awaitUninterruptibly
import com.swordfish.libretrodroid.gamepad.GamepadsManager
import kotlinx.coroutines.CoroutineExceptionHandler
import kotlinx.coroutines.GlobalScope
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.flow.MutableSharedFlow
import kotlinx.coroutines.launch
import java.util.Locale
import java.util.concurrent.CountDownLatch
import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.opengles.GL10
import kotlin.properties.Delegates

class GLRetroView(
    context: Context,
    private val data: GLRetroViewData
) : GLSurfaceView(context), LifecycleObserver {

    private val handler = CoroutineExceptionHandler { _, exception ->
        Log.d("DQC", " ---------- CoroutineExceptionHandler in retro view $exception ")
    }

    var audioEnabled: Boolean by Delegates.observable(true) { _, _, value ->
        catchExceptions {
            LibretroDroid.setAudioEnabled(value)
        }
    }

    var frameSpeed: Int by Delegates.observable(1) { _, _, value ->
        catchExceptions {
            LibretroDroid.setFrameSpeed(value)
        }
    }

    var shader: ShaderConfig by Delegates.observable(data.shader) { _, _, value ->
        catchExceptions {
            LibretroDroid.setShaderConfig(buildShader(value))
        }
    }

    var viewport: RectF by Delegates.observable(RectF(0f, 0f, 1f, 1f)) { _, _, value ->
        catchExceptions {
            LibretroDroid.setViewport(value.left, value.top, value.width(), value.height())
        }
    }

    private var openGLESVersion: Int = 3

    private var isGameLoaded = false
    private var isEmulationReady = false
    private var isAborted = false

    private val retroGLEventsSubject = MutableSharedFlow<GLRetroEvents>(1)
    private val retroGLIssuesErrors = MutableSharedFlow<Int>(1)

    private val rumbleEventsSubject = MutableSharedFlow<RumbleEvent>()

    private var lifecycle: Lifecycle? = null

    init {
        catchExceptions {
            openGLESVersion = getGLESVersion(context)
            preserveEGLContextOnPause = true
            setEGLContextClientVersion(openGLESVersion)
            setRenderer(Renderer())
            keepScreenOn = true
        }
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
            buildShader(data.shader),
            getDefaultRefreshRate(),
            data.preferLowLatencyAudio,
            data.gameVirtualFiles.isNotEmpty(),
            data.enableMicrophone,
            data.skipDuplicateFrames,
            data.enableAmbientMode,
            getDeviceLanguage()
        )
        LibretroDroid.setRumbleEnabled(data.rumbleEventsEnabled)
    }

    @OnLifecycleEvent(Lifecycle.Event.ON_DESTROY)
    fun onDestroy() = catchExceptions {
        LibretroDroid.destroy()
        lifecycle = null
    }

    private fun getDeviceLanguage() = catchExceptionsWithResult {
        Locale.getDefault().language
    } ?: ""

    private fun getDefaultRefreshRate() = catchExceptionsWithResult {
        (context.getSystemService(Context.WINDOW_SERVICE) as WindowManager).defaultDisplay.refreshRate
    } ?: 0f

    fun sendKeyEvent(action: Int, keyCode: Int, port: Int = 0) = catchExceptions {
        queueEvent { LibretroDroid.onKeyEvent(port, action, keyCode) }
    }

    fun sendMotionEvent(source: Int, xAxis: Float, yAxis: Float, port: Int = 0) = catchExceptions {
        queueEvent { LibretroDroid.onMotionEvent(port, source, xAxis, yAxis) }
    }

    override fun onTouchEvent(event: MotionEvent?): Boolean {
        catchExceptions {
            when (event?.actionMasked) {
                MotionEvent.ACTION_DOWN, MotionEvent.ACTION_MOVE -> normalizeTouchCoordinates(event.x, event.y)
                MotionEvent.ACTION_UP -> TOUCH_EVENT_OUTSIDE
                else -> null
            }?.let { position ->
                LibretroDroid.onTouchEvent(position.x, position.y)
            }
        }
        return true
    }

    private fun clamp(x: Float, min: Float, max: Float) = catchExceptionsWithResult {
        minOf(maxOf(x, min), max)
    } ?: 0f

    private fun normalizeTouchCoordinates(x: Float, y: Float): PointF {
        return catchExceptionsWithResult {
            val x = clamp(2f * x / width - 1f, -1f, +1f)
            val y = clamp(2f * y / height - 1f, -1f, +1f)
            PointF(x, y)
        } ?: PointF(0f, 0f)
    }

    fun serializeState(): ByteArray = runOnGLThread {
        LibretroDroid.serializeState()
    } ?: byteArrayOf()

    fun setCheat(index: Int, enable: Boolean, code: String) = runOnGLThread {
        LibretroDroid.setCheat(index, enable, code)
    }

    fun unserializeState(data: ByteArray): Boolean = runOnGLThread {
        LibretroDroid.unserializeState(data)
    } == true

    fun serializeSRAM(): ByteArray = runOnGLThread {
        LibretroDroid.serializeSRAM()
    } ?: byteArrayOf()

    fun unserializeSRAM(data: ByteArray): Boolean = runOnGLThread {
        LibretroDroid.unserializeSRAM(data)
    } == true

    fun reset() = runOnGLThread {
        LibretroDroid.reset()
    }

    fun getGLRetroEvents(): Flow<GLRetroEvents> {
        return retroGLEventsSubject
    }

    fun getGLRetroErrors(): Flow<Int> {
        return retroGLIssuesErrors
    }

    fun getRumbleEvents(): Flow<RumbleEvent> {
        return rumbleEventsSubject
    }

    fun getControllers(): Array<Array<Controller>> {
        return catchExceptionsWithResult {
            LibretroDroid.getControllers()
        } ?: arrayOf(arrayOf())
    }

    fun setControllerType(port: Int, type: Int) {
        catchExceptions {
            LibretroDroid.setControllerType(port, type)
        }
    }

    fun getVariables(): Array<Variable> {
        return catchExceptionsWithResult {
            LibretroDroid.getVariables()
        } ?: arrayOf()
    }

    fun updateVariables(vararg variables: Variable) {
        catchExceptions {
            variables.forEach {
                LibretroDroid.updateVariable(it)
            }
        }
    }

    fun getAvailableDisks() = runOnGLThread { LibretroDroid.availableDisks() } ?: 0

    fun getCurrentDisk() = runOnGLThread { LibretroDroid.currentDisk() } ?: 0

    fun changeDisk(index: Int) = runOnGLThread { LibretroDroid.changeDisk(index) }

    private fun getGLESVersion(context: Context): Int {
        val activityManager = context.getSystemService(Context.ACTIVITY_SERVICE) as ActivityManager
        return if (activityManager.deviceConfigurationInfo.reqGlEsVersion >= 0x30000) {
            3
        } else {
            2
        }
    }

    override fun onKeyDown(keyCode: Int, event: KeyEvent?): Boolean {
       return catchExceptionsWithResult {
           val mappedKey = GamepadsManager.getGamepadKeyEvent(keyCode)
           val port = (event?.device?.controllerNumber ?: 0) - 1

           if (event != null && port >= 0 && keyCode in GamepadsManager.GAMEPAD_KEYS) {
               sendKeyEvent(KeyEvent.ACTION_DOWN, mappedKey, port)
               true
           }
           super.onKeyDown(keyCode, event)
       } == true
    }

    override fun onKeyUp(keyCode: Int, event: KeyEvent?): Boolean {
        return catchExceptionsWithResult {
            val mappedKey = GamepadsManager.getGamepadKeyEvent(keyCode)
            val port = (event?.device?.controllerNumber ?: 0) - 1

            if (event != null && port >= 0 && keyCode in GamepadsManager.GAMEPAD_KEYS) {
                sendKeyEvent(KeyEvent.ACTION_UP, mappedKey, port)
                 true
            }
             super.onKeyUp(keyCode, event)
        } == true
    }

    override fun onGenericMotionEvent(event: MotionEvent?): Boolean {
        return catchExceptionsWithResult {
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
            super.onGenericMotionEvent(event)
        } == true
    }

    // These functions are called only after the GLSurfaceView has been created.
    private inner class RenderLifecycleObserver : LifecycleObserver {
        @OnLifecycleEvent(Lifecycle.Event.ON_RESUME)
        private fun resume() = catchExceptions {
            LibretroDroid.resume()
            onResume()
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
                lifecycle?.coroutineScope?.launch {
                    retroGLEventsSubject.emit(GLRetroEvents.FrameRendered)
                }
            }
        }

        override fun onSurfaceChanged(gl: GL10, width: Int, height: Int) = catchExceptions {
            Thread.currentThread().priority = Thread.MAX_PRIORITY
            LibretroDroid.onSurfaceChanged(width, height)
        }


        override fun onSurfaceCreated(gl: GL10, config: EGLConfig) = catchExceptions {
            Thread.currentThread().priority = Thread.MAX_PRIORITY
            initializeCore()
            lifecycle?.coroutineScope?.launch {
                retroGLEventsSubject.emit(GLRetroEvents.SurfaceCreated)
            }
        }
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
        catchExceptions {
            val detachedVirtualFiles = virtualFiles
                .map { DetachedVirtualFile(it.virtualPath, it.fileDescriptor.detachFd()) }
            LibretroDroid.loadGameFromVirtualFiles(detachedVirtualFiles)
        }
    }

    private fun loadGameFromBytes(gameFileBytes: ByteArray) {
        catchExceptions {
            LibretroDroid.loadGameFromBytes(gameFileBytes)
        }
    }

    private fun loadGameFromPath(gameFilePath: String) {
        catchExceptions {
            LibretroDroid.loadGameFromPath(gameFilePath)
        }
    }

    private fun catchExceptions(block: () -> Unit) {
        try {
            if (isAborted) return
            block()
        } catch (e: RetroException) {
            GlobalScope.launch {
                retroGLIssuesErrors.emit(e.errorCode)
            }
            isAborted = true
        } catch (e: Exception) {
            Log.e(TAG_LOG, "Error in GLRetroView", e)
            GlobalScope.launch {
                retroGLIssuesErrors.emit(LibretroDroid.ERROR_GENERIC)
            }
            isAborted = true
        }
    }

    private fun <T> catchExceptionsWithResult(block: () -> T?): T? {
        return try {
            if (isAborted) return null
            block()
        } catch (e: RetroException) {
            GlobalScope.launch {
                retroGLIssuesErrors.emit(e.errorCode)
            }
            isAborted = true
            null
        } catch (e: Exception) {
            Log.e(TAG_LOG, "Error in GLRetroView", e)
            GlobalScope.launch {
                retroGLIssuesErrors.emit(LibretroDroid.ERROR_GENERIC)
            }
            isAborted = true
            null
        }
    }

    private fun <T> runOnGLThread(block: () -> T): T? {
        return catchExceptionsWithResult {
            if (Thread.currentThread().name.startsWith("GLThread")) {
                return@catchExceptionsWithResult block()
            }

            val latch = CountDownLatch(1)
            var result: T? = null
            queueEvent {
                result = block()
                latch.countDown()
            }

            latch.awaitUninterruptibly()
            result
        }
    }

    private fun buildShader(config: ShaderConfig): GLRetroShader {
        return when (config) {
            is ShaderConfig.Default -> GLRetroShader(LibretroDroid.SHADER_DEFAULT)
            is ShaderConfig.CRT -> GLRetroShader(LibretroDroid.SHADER_CRT)
            is ShaderConfig.LCD -> GLRetroShader(LibretroDroid.SHADER_LCD)
            is ShaderConfig.Sharp -> GLRetroShader(LibretroDroid.SHADER_SHARP)
            is ShaderConfig.CUT -> GLRetroShader(
                LibretroDroid.SHADER_UPSCALE_CUT,
                buildParams(
                    LibretroDroid.SHADER_UPSCALE_CUT_PARAM_USE_DYNAMIC_BLEND to toParam(config.useDynamicBlend),
                    LibretroDroid.SHADER_UPSCALE_CUT_PARAM_BLEND_MIN_CONTRAST_EDGE to toParam(config.blendMinContrastEdge),
                    LibretroDroid.SHADER_UPSCALE_CUT_PARAM_BLEND_MAX_CONTRAST_EDGE to toParam(config.blendMaxContrastEdge),
                    LibretroDroid.SHADER_UPSCALE_CUT_PARAM_BLEND_MIN_SHARPNESS to toParam(config.blendMinSharpness),
                    LibretroDroid.SHADER_UPSCALE_CUT_PARAM_BLEND_MAX_SHARPNESS to toParam(config.blendMaxSharpness),
                    LibretroDroid.SHADER_UPSCALE_CUT_PARAM_STATIC_BLEND_SHARPNESS to toParam(config.staticSharpness),
                    LibretroDroid.SHADER_UPSCALE_CUT_PARAM_EDGE_USE_FAST_LUMA to toParam(config.edgeUseFastLuma),
                    LibretroDroid.SHADER_UPSCALE_CUT_PARAM_EDGE_MIN_VALUE to toParam(config.edgeMinValue),
                    LibretroDroid.SHADER_UPSCALE_CUT_PARAM_EDGE_MIN_CONTRAST to toParam(config.edgeMinContrast),
                )
            )

            is ShaderConfig.CUT2 -> GLRetroShader(
                LibretroDroid.SHADER_UPSCALE_CUT2,
                buildParams(
                    LibretroDroid.SHADER_UPSCALE_CUT2_PARAM_USE_DYNAMIC_BLEND to toParam(config.useDynamicBlend),
                    LibretroDroid.SHADER_UPSCALE_CUT2_PARAM_BLEND_MIN_CONTRAST_EDGE to toParam(config.blendMinContrastEdge),
                    LibretroDroid.SHADER_UPSCALE_CUT2_PARAM_BLEND_MAX_CONTRAST_EDGE to toParam(config.blendMaxContrastEdge),
                    LibretroDroid.SHADER_UPSCALE_CUT2_PARAM_BLEND_MIN_SHARPNESS to toParam(config.blendMinSharpness),
                    LibretroDroid.SHADER_UPSCALE_CUT2_PARAM_BLEND_MAX_SHARPNESS to toParam(config.blendMaxSharpness),
                    LibretroDroid.SHADER_UPSCALE_CUT2_PARAM_STATIC_BLEND_SHARPNESS to toParam(config.staticSharpness),
                    LibretroDroid.SHADER_UPSCALE_CUT2_PARAM_EDGE_USE_FAST_LUMA to toParam(config.edgeUseFastLuma),
                    LibretroDroid.SHADER_UPSCALE_CUT2_PARAM_EDGE_MIN_VALUE to toParam(config.edgeMinValue),
                    LibretroDroid.SHADER_UPSCALE_CUT2_PARAM_SOFT_EDGES_SHARPENING to toParam(config.softEdgesSharpening),
                    LibretroDroid.SHADER_UPSCALE_CUT2_PARAM_SOFT_EDGES_SHARPENING_AMOUNT to toParam(config.softEdgesSharpeningAmount),
                    LibretroDroid.SHADER_UPSCALE_CUT2_PARAM_HARD_EDGES_THRESHOLD to toParam(config.hardEdgesThreshold),
                )
            )

            is ShaderConfig.CUT3 -> GLRetroShader(
                LibretroDroid.SHADER_UPSCALE_CUT3,
                buildParams(
                    LibretroDroid.SHADER_UPSCALE_CUT3_PARAM_USE_DYNAMIC_BLEND to toParam(config.useDynamicBlend),
                    LibretroDroid.SHADER_UPSCALE_CUT3_PARAM_BLEND_MIN_CONTRAST_EDGE to toParam(config.blendMinContrastEdge),
                    LibretroDroid.SHADER_UPSCALE_CUT3_PARAM_BLEND_MAX_CONTRAST_EDGE to toParam(config.blendMaxContrastEdge),
                    LibretroDroid.SHADER_UPSCALE_CUT3_PARAM_BLEND_MIN_SHARPNESS to toParam(config.blendMinSharpness),
                    LibretroDroid.SHADER_UPSCALE_CUT3_PARAM_BLEND_MAX_SHARPNESS to toParam(config.blendMaxSharpness),
                    LibretroDroid.SHADER_UPSCALE_CUT3_PARAM_STATIC_BLEND_SHARPNESS to toParam(config.staticSharpness),
                    LibretroDroid.SHADER_UPSCALE_CUT3_PARAM_EDGE_USE_FAST_LUMA to toParam(config.edgeUseFastLuma),
                    LibretroDroid.SHADER_UPSCALE_CUT3_PARAM_EDGE_MIN_VALUE to toParam(config.edgeMinValue),
                    LibretroDroid.SHADER_UPSCALE_CUT3_PARAM_SOFT_EDGES_SHARPENING to toParam(config.softEdgesSharpening),
                    LibretroDroid.SHADER_UPSCALE_CUT3_PARAM_SOFT_EDGES_SHARPENING_AMOUNT to toParam(config.softEdgesSharpeningAmount),
                    LibretroDroid.SHADER_UPSCALE_CUT3_PARAM_HARD_EDGES_THRESHOLD to toParam(config.hardEdgesThreshold),
                    LibretroDroid.SHADER_UPSCALE_CUT3_PARAM_HARD_EDGES_SEARCH_MIN_CONTRAST to toParam(config.hardEdgesSearchMinContrast),
                    LibretroDroid.SHADER_UPSCALE_CUT3_PARAM_HARD_EDGES_SEARCH_MAX_DISTANCE to toParam(config.hardEdgesSearchMaxDistance),
                )
            )
        }
    }

    private fun toParam(param: Float): String {
        return param.toString()
    }

    private fun toParam(param: Boolean): String {
        return if (param) {
            "1"
        } else {
            "0"
        }
    }

    private fun toParam(param: Int): String {
        return param.toString()
    }

    private fun buildParams(vararg pairs: Pair<String, String?>): Map<String, String> {
        return pairs
            .filter { (key, value) -> value != null }
            .associate { (key, value) -> key to value!! }
    }

    /** This function gets called from the jni side.*/
    private fun sendRumbleEvent(port: Int, strengthWeak: Float, strengthStrong: Float) {
        lifecycle?.coroutineScope?.launch(handler) {
            rumbleEventsSubject.emit(RumbleEvent(port, strengthWeak, strengthStrong))
        }
    }

    private fun refreshAspectRatio() {
        runOnGLThread {
            LibretroDroid.refreshAspectRatio()
        }
    }

    sealed class GLRetroEvents {
        object FrameRendered : GLRetroEvents()
        object SurfaceCreated : GLRetroEvents()
    }

    companion object {
        private val TAG_LOG = GLRetroView::class.java.simpleName

        const val MOTION_SOURCE_DPAD = LibretroDroid.MOTION_SOURCE_DPAD
        const val MOTION_SOURCE_ANALOG_LEFT = LibretroDroid.MOTION_SOURCE_ANALOG_LEFT
        const val MOTION_SOURCE_ANALOG_RIGHT = LibretroDroid.MOTION_SOURCE_ANALOG_RIGHT
        const val MOTION_SOURCE_POINTER = LibretroDroid.MOTION_SOURCE_POINTER

        const val ERROR_LOAD_LIBRARY = LibretroDroid.ERROR_LOAD_LIBRARY
        const val ERROR_LOAD_GAME = LibretroDroid.ERROR_LOAD_GAME
        const val ERROR_GL_NOT_COMPATIBLE = LibretroDroid.ERROR_GL_NOT_COMPATIBLE
        const val ERROR_SERIALIZATION = LibretroDroid.ERROR_SERIALIZATION
        const val ERROR_CHEAT = LibretroDroid.ERROR_CHEAT
        const val ERROR_GENERIC = LibretroDroid.ERROR_GENERIC

        private val TOUCH_EVENT_OUTSIDE = PointF(-10f, 10f)
    }
}
