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
import android.view.*
import com.jakewharton.rxrelay2.PublishRelay
import com.swordfish.libretrodroid.gamepad.GamepadInfo
import com.swordfish.libretrodroid.gamepad.GamepadsManager
import io.reactivex.Observable
import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.opengles.GL10

class GLRetroView(context: Context,
    private val coreFilePath: String,
    private val gameFilePath: String,
    private val systemDirectory: String = context.filesDir.absolutePath,
    private val savesDirectory: String = context.filesDir.absolutePath,
    private val shader: Int = LibretroDroid.SHADER_DEFAULT
) : AspectRatioGLSurfaceView(context) {

    private val openGLESVersion: Int
    private val gamepadsManager = GamepadsManager(context.applicationContext)
    private val keyEventSubject = PublishRelay.create<GameKeyEvent>()
    private val motionEventSubject = PublishRelay.create<GameMotionEvent>()

    init {
        openGLESVersion = getGLESVersion(context)
        preserveEGLContextOnPause = true
        setEGLContextClientVersion(openGLESVersion)
        setRenderer(Renderer())
        keepScreenOn = true
        isFocusable = true
    }

    fun onCreate() {
        LibretroDroid.create(
            openGLESVersion,
            coreFilePath,
            gameFilePath,
            systemDirectory,
            savesDirectory,
            shader,
            getScreenRefreshRate()
        )

        gamepadsManager.init()
        setAspectRatio(LibretroDroid.getAspectRatio())
    }

    private fun getScreenRefreshRate(): Float {
        return (context.getSystemService(Context.WINDOW_SERVICE) as WindowManager).defaultDisplay.refreshRate
    }

    override fun onResume() {
        LibretroDroid.resume()
        super.onResume()
    }

    fun getAspectRatio(): Float {
        return LibretroDroid.getAspectRatio()
    }

    override fun onPause() {
        super.onPause()
        LibretroDroid.pause()
    }

    fun onDestroy() {
        LibretroDroid.destroy()
        gamepadsManager.deinit()
    }

    fun sendKeyEvent(action: Int, keyCode: Int, port: Int = 0) {
        queueEvent { LibretroDroid.onKeyEvent(port, action, keyCode) }
        keyEventSubject.accept(GameKeyEvent(action, keyCode, port))
    }

    fun sendMotionEvent(source: Int, xAxis: Float, yAxis: Float, port: Int = 0) {
        queueEvent { LibretroDroid.onMotionEvent(port, source, xAxis, yAxis) }
        motionEventSubject.accept(GameMotionEvent(source, xAxis, yAxis, port))
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

    private fun sendTouchEvent(event: MotionEvent) {
        val x = event.x / width
        val y = event.y / height
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

    fun getGamepadInfos(): Observable<List<GamepadInfo>> {
        return gamepadsManager.getGamepadInfos()
    }

    fun getGameKeyEvents(): Observable<GameKeyEvent> {
        return keyEventSubject
    }

    fun getGameMotionEvents(): Observable<GameMotionEvent> {
        return motionEventSubject
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

    override fun onKeyDown(originalKeyCode: Int, event: KeyEvent): Boolean {
        val keyCode = gamepadsManager.getGamepadKeyEvent(originalKeyCode)
        val port = gamepadsManager.getGamepadPort(event.deviceId)

        if (keyCode in GamepadInfo.GAMEPAD_KEYS) {
            sendKeyEvent(KeyEvent.ACTION_DOWN, keyCode, port)
            return true
        }
        return false
    }

    override fun onKeyUp(originalKeyCode: Int, event: KeyEvent): Boolean {
        val keyCode = gamepadsManager.getGamepadKeyEvent(originalKeyCode)
        val port = gamepadsManager.getGamepadPort(event.deviceId)

        if (keyCode in GamepadInfo.GAMEPAD_KEYS) {
            sendKeyEvent(KeyEvent.ACTION_UP, keyCode, port)
            return true
        }
        return false
    }

    override fun onGenericMotionEvent(event: MotionEvent): Boolean {
        val port = gamepadsManager.getGamepadPort(event.deviceId)
        when (event.source) {
            InputDevice.SOURCE_JOYSTICK -> {
                sendMotionEvent(MOTION_SOURCE_DPAD, event.getAxisValue(MotionEvent.AXIS_HAT_X), event.getAxisValue(MotionEvent.AXIS_HAT_Y), port)
                sendMotionEvent(MOTION_SOURCE_ANALOG_LEFT, event.getAxisValue(MotionEvent.AXIS_X), event.getAxisValue(MotionEvent.AXIS_Y), port)
                sendMotionEvent(MOTION_SOURCE_ANALOG_RIGHT, event.getAxisValue(MotionEvent.AXIS_Z), event.getAxisValue(MotionEvent.AXIS_RZ), port)
            }
        }
        return super.onGenericMotionEvent(event)
    }

    private class Renderer : GLSurfaceView.Renderer {
        override fun onDrawFrame(gl: GL10) {
            LibretroDroid.step()
        }

        override fun onSurfaceChanged(gl: GL10, width: Int, height: Int) {
            LibretroDroid.onSurfaceChanged(width, height)
        }

        override fun onSurfaceCreated(gl: GL10, config: EGLConfig) {
            LibretroDroid.onSurfaceCreated()
        }
    }

    data class GameKeyEvent(val action: Int, val keyCode: Int, val port: Int)
    data class GameMotionEvent(val source: Int, val xAxis: Float, val yAxis: Float, val port: Int)

    companion object {
        const val MOTION_SOURCE_DPAD = LibretroDroid.MOTION_SOURCE_DPAD
        const val MOTION_SOURCE_ANALOG_LEFT = LibretroDroid.MOTION_SOURCE_ANALOG_LEFT
        const val MOTION_SOURCE_ANALOG_RIGHT = LibretroDroid.MOTION_SOURCE_ANALOG_RIGHT
        const val MOTION_SOURCE_POINTER = LibretroDroid.MOTION_SOURCE_POINTER

        const val SHADER_DEFAULT = LibretroDroid.SHADER_DEFAULT
        const val SHADER_CRT = LibretroDroid.SHADER_CRT
        const val SHADER_LCD = LibretroDroid.SHADER_LCD
    }
}
