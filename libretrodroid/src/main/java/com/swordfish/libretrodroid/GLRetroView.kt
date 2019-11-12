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
import android.util.Log
import android.view.InputDevice
import android.view.KeyEvent
import android.view.MotionEvent

import javax.microedition.khronos.egl.EGL10
import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.egl.EGLContext
import javax.microedition.khronos.egl.EGLDisplay
import javax.microedition.khronos.opengles.GL10

class GLRetroView(context: Context) : GLSurfaceView(context) {

    init {
        preserveEGLContextOnPause = true
        setEGLConfigChooser(5, 6, 5, 0, 0, 0)
        setEGLContextClientVersion(3)
        setRenderer(Renderer())
        keepScreenOn = true
    }

    fun onCreate(coreFilePath: String, gameFilePath: String) {
        LibretroDroid.create(coreFilePath, gameFilePath)
    }

    override fun onPause() {
        LibretroDroid.pause()
        super.onPause()
    }

    override fun onResume() {
        super.onResume()
        LibretroDroid.resume()
    }

    fun onDestroy() {
        LibretroDroid.destroy()
    }

    override fun onKeyDown(keyCode: Int, event: KeyEvent): Boolean {
        return !LibretroDroid.onKeyEvent(KeyEvent.ACTION_DOWN, keyCode)
    }

    override fun onKeyUp(keyCode: Int, event: KeyEvent): Boolean {
        return !LibretroDroid.onKeyEvent(KeyEvent.ACTION_UP, keyCode)
    }

    override fun onGenericMotionEvent(event: MotionEvent): Boolean {
        when (event.source) {
            InputDevice.SOURCE_JOYSTICK -> {
                LibretroDroid.onMotionEvent(LibretroDroid.MOTION_SOURCE_DPAD, event.getAxisValue(MotionEvent.AXIS_HAT_X), event.getAxisValue(MotionEvent.AXIS_HAT_Y))
                LibretroDroid.onMotionEvent(LibretroDroid.MOTION_SOURCE_ANALOG_LEFT, event.getAxisValue(MotionEvent.AXIS_X), event.getAxisValue(MotionEvent.AXIS_Y))
                LibretroDroid.onMotionEvent(LibretroDroid.MOTION_SOURCE_ANALOG_RIGHT, event.getAxisValue(MotionEvent.AXIS_Z), event.getAxisValue(MotionEvent.AXIS_RZ))
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
}
