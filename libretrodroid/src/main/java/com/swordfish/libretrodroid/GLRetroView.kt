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

import android.content.Context
import android.graphics.PointF
import android.graphics.RectF
import android.view.InputDevice
import android.view.KeyEvent
import android.view.MotionEvent
import android.view.SurfaceView
import androidx.annotation.MainThread
import androidx.lifecycle.Lifecycle
import androidx.lifecycle.LifecycleObserver
import androidx.lifecycle.LifecycleOwner
import androidx.lifecycle.OnLifecycleEvent
import com.swordfish.libretrodroid.gamepad.GamepadsManager
import com.swordfish.libretrodroid.session.LibretroDroidSession
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.runBlocking
import kotlinx.coroutines.flow.map

class GLRetroView(
    context: Context,
    data: GLRetroViewData,
) : SurfaceView(context), LifecycleObserver {

    private val session = LibretroDroidSession(data.toSessionConfig(context))

    var audioEnabled: Boolean
        get() { return session.audioEnabled }
        set(value) { session.audioEnabled = value }

    var frameSpeed: Int
        get() { return session.frameSpeed }
        set(value) { session.frameSpeed = value }

    var shader: ShaderConfig
        get() { return session.shader }
        set(value) { session.shader = value }

    var viewport: RectF
        get() { return session.viewport }
        set(value) { session.viewport = value }

    var viewportAlignment: ViewportAlignment
        get() { return session.viewportAlignment }
        set(value) { session.viewportAlignment = value }

    init {
        keepScreenOn = true
    }

    @MainThread
    @OnLifecycleEvent(Lifecycle.Event.ON_CREATE)
    fun onCreate(lifecycleOwner: LifecycleOwner) {
        session.create()
        session.attach(this)
    }

    @MainThread
    @OnLifecycleEvent(Lifecycle.Event.ON_DESTROY)
    fun onDestroy() {
        session.destroy()
    }

    @MainThread
    @OnLifecycleEvent(Lifecycle.Event.ON_RESUME)
    fun onResume() {
        session.resume()
    }

    @MainThread
    @OnLifecycleEvent(Lifecycle.Event.ON_PAUSE)
    fun onPause() {
        session.pause()
    }

    fun queueEvent(event: Runnable) {
        session.dispatch { event.run() }
    }

    fun sendKeyEvent(action: Int, keyCode: Int, port: Int = 0) {
        session.sendKeyEvent(action, keyCode, port)
    }

    fun sendMotionEvent(source: Int, xAxis: Float, yAxis: Float, port: Int = 0) {
        session.sendMotionEvent(source, xAxis, yAxis, port)
    }

    override fun onTouchEvent(event: MotionEvent?): Boolean {
        val position = when (event?.actionMasked) {
            MotionEvent.ACTION_DOWN, MotionEvent.ACTION_MOVE -> {
                normalizeTouchCoordinates(event.x, event.y)
            }

            MotionEvent.ACTION_UP -> {
                TOUCH_EVENT_OUTSIDE
            }

            else -> null
        }

        if (position != null) {
            session.sendTouchEvent(position.x, position.y)
        }

        return true
    }

    private fun normalizeTouchCoordinates(x: Float, y: Float): PointF {
        return PointF(
            (2f * x / width - 1f).coerceIn(-1f, 1f),
            (2f * y / height - 1f).coerceIn(-1f, 1f),
        )
    }

    fun serializeState(useEmulationThread: Boolean = true): ByteArray {
        return runBlocking { session.serializeState() }
    }

    fun unserializeState(data: ByteArray, useEmulationThread: Boolean = true): Boolean {
        return runBlocking { session.unserializeState(data) }
    }

    fun serializeSRAM(useEmulationThread: Boolean = true): ByteArray {
        return runBlocking { session.serializeSRAM() }
    }

    fun unserializeSRAM(data: ByteArray, useEmulationThread: Boolean = true): Boolean {
        return runBlocking { session.unserializeSRAM(data) }
    }

    fun setCheat(index: Int, enable: Boolean, code: String, useEmulationThread: Boolean = true) {
        session.setCheat(index, enable, code)
    }

    fun reset(useEmulationThread: Boolean = true) {
        session.reset()
    }

    fun getControllers(): Array<Array<Controller>> {
        return runBlocking { session.getControllers() }
    }

    fun setControllerType(port: Int, type: Int) {
        session.setControllerType(port, type)
    }

    fun getVariables(): Array<Variable> {
        return runBlocking { session.getVariables() }
    }

    fun updateVariables(vararg variables: Variable) {
        session.updateVariables(*variables)
    }

    fun getAvailableDisks(useEmulationThread: Boolean = true): Int {
        return runBlocking { session.getAvailableDisks() }
    }

    fun getCurrentDisk(useEmulationThread: Boolean = true): Int {
        return runBlocking { session.getCurrentDisk() }
    }

    fun changeDisk(index: Int, useEmulationThread: Boolean = true) {
        session.changeDisk(index)
    }

    fun getGLRetroEvents(): Flow<GLRetroEvents> {
        return session.events.map {
            when (it) {
                LibretroDroidSession.Event.Created -> GLRetroEvents.SurfaceCreated
            }
        }
    }

    fun getGLRetroErrors(): Flow<Int> {
        return session.errors
    }

    fun getRumbleEvents(): Flow<RumbleEvent> {
        return session.rumbleEvents
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

    sealed class GLRetroEvents {
        // TODO This should be deleted so that we can also tear down all the logic that handles GLRetroEvents in general
        object SurfaceCreated : GLRetroEvents()
    }

    companion object {
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
