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

import android.content.Context
import android.opengl.GLSurfaceView
import android.view.*
import com.jakewharton.rxrelay2.PublishRelay
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

    private val gamepadsManager = GamepadsManager(context.applicationContext)
    private val keyEventSubject = PublishRelay.create<GameKeyEvent>()
    private val motionEventSubject = PublishRelay.create<GameMotionEvent>()

    init {
        preserveEGLContextOnPause = true
        setEGLConfigChooser(5, 6, 5, 0, 0, 0)
        setEGLContextClientVersion(3)
        setRenderer(Renderer())
        keepScreenOn = true
        isFocusable = true
    }

    fun onCreate() {
        LibretroDroid.create(
            coreFilePath,
            gameFilePath,
            systemDirectory,
            savesDirectory,
            shader,
            getScreenRefreshRate())

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

    fun getConnectedGamepads(): Observable<Int> {
        return gamepadsManager.getConnectedGamepads()
    }

    fun getGameKeyEvents(): Observable<GameKeyEvent> {
        return keyEventSubject
    }

    fun getGameMotionEvents(): Observable<GameMotionEvent> {
        return motionEventSubject
    }

    override fun onKeyDown(originalKeyCode: Int, event: KeyEvent): Boolean {
        val keyCode = gamepadsManager.getGamepadKeyEvent(originalKeyCode)
        val port = gamepadsManager.getGamepadPort(event.deviceId)

        if (keyCode in GAMEPAD_KEYS) {
            sendKeyEvent(KeyEvent.ACTION_DOWN, keyCode, port)
            return true
        }
        return false
    }

    override fun onKeyUp(originalKeyCode: Int, event: KeyEvent): Boolean {
        val keyCode = gamepadsManager.getGamepadKeyEvent(originalKeyCode)
        val port = gamepadsManager.getGamepadPort(event.deviceId)

        if (keyCode in GAMEPAD_KEYS) {
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

        private val GAMEPAD_KEYS = setOf(
                KeyEvent.KEYCODE_BUTTON_SELECT,
                KeyEvent.KEYCODE_BUTTON_START,
                KeyEvent.KEYCODE_BUTTON_A,
                KeyEvent.KEYCODE_BUTTON_X,
                KeyEvent.KEYCODE_BUTTON_Y,
                KeyEvent.KEYCODE_BUTTON_B,
                KeyEvent.KEYCODE_BUTTON_L1,
                KeyEvent.KEYCODE_BUTTON_L2,
                KeyEvent.KEYCODE_BUTTON_R1,
                KeyEvent.KEYCODE_BUTTON_R2,
                KeyEvent.KEYCODE_BUTTON_THUMBL,
                KeyEvent.KEYCODE_BUTTON_THUMBR
        )
    }
}
