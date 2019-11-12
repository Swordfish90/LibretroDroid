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

package com.swordfish.libretrodroid;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.util.Log;
import android.view.InputDevice;
import android.view.KeyEvent;
import android.view.MotionEvent;

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLContext;
import javax.microedition.khronos.egl.EGLDisplay;
import javax.microedition.khronos.opengles.GL10;

class GLRetroView extends GLSurfaceView {

    public GLRetroView(Context context) {
        super(context);
        init();
    }

    @Override
    public void onPause() {
        LibretroDroid.pause();
        super.onPause();
    }

    @Override
    public void onResume() {
        super.onResume();
        LibretroDroid.resume();
    }

    public void onDestroy() {
        LibretroDroid.destroy();
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        return !LibretroDroid.onKeyEvent(KeyEvent.ACTION_DOWN, keyCode);
    }

    @Override
    public boolean onKeyUp(int keyCode, KeyEvent event) {
        return !LibretroDroid.onKeyEvent(KeyEvent.ACTION_UP, keyCode);
    }

    @Override
    public boolean onGenericMotionEvent(MotionEvent event) {
        switch(event.getSource()) {
            case InputDevice.SOURCE_JOYSTICK:
                LibretroDroid.onMotionEvent(LibretroDroid.MOTION_SOURCE_DPAD, event.getAxisValue(MotionEvent.AXIS_HAT_X), event.getAxisValue(MotionEvent.AXIS_HAT_Y));
                LibretroDroid.onMotionEvent(LibretroDroid.MOTION_SOURCE_ANALOG_LEFT, event.getAxisValue(MotionEvent.AXIS_X), event.getAxisValue(MotionEvent.AXIS_Y));
                LibretroDroid.onMotionEvent(LibretroDroid.MOTION_SOURCE_ANALOG_RIGHT, event.getAxisValue(MotionEvent.AXIS_Z), event.getAxisValue(MotionEvent.AXIS_RZ));
                break;
        }
        return super.onGenericMotionEvent(event);
    }

    private void init() {
        /* Set the renderer responsible for frame rendering */

        //LibretroDroid.create("gambatte_libretro_android.so", "/storage/emulated/0/Roms Test/gb/Super Mario Land.gb");
        LibretroDroid.create("gambatte_libretro_android.so", "/storage/emulated/0/Roms Test/gb/Pokemon Blue Version.gb");
        //LibretroDroid.create("mgba_libretro_android.so", "/storage/emulated/0/Roms Test/gba/Advance Wars.gba");
        //LibretroDroid.create("gambatte_libretro_android.so", "/storage/emulated/0/Roms Test/gb/Super Mario Land 2 - 6 Golden Coins.gb");
        //LibretroDroid.create("mgba_libretro_android.so", "/storage/emulated/0/Roms Test/gb/Tetris.gb");
        //LibretroDroid.create("mupen64plus_next_gles3_libretro_android.so", "/storage/emulated/0/Roms Test/n64/Super Mario 64/Super Mario 64.n64");
        //LibretroDroid.create("mupen64plus_next_gles3_libretro_android.so", "/storage/emulated/0/Roms Test/n64/Legend of Zelda, The - Ocarina of Time - Master Quest/Legend of Zelda, The - Ocarina of Time - Master Quest.z64");

        // Disabling this leads to crashes when using mupen64plus.
        // We should consider migrating to SurfaceView and manually manage egl contexts.
        setPreserveEGLContextOnPause(true);
        setEGLConfigChooser(5, 6, 5, 0, 0, 0);
        setEGLContextClientVersion(3);
        setRenderer(new Renderer());
        setKeepScreenOn(true);
    }

    private static class Renderer implements GLSurfaceView.Renderer {
        public void onDrawFrame(GL10 gl) {
            LibretroDroid.step();
        }

        public void onSurfaceChanged(GL10 gl, int width, int height) {
            LibretroDroid.onSurfaceChanged(width, height);
        }

        public void onSurfaceCreated(GL10 gl, EGLConfig config) {
            LibretroDroid.onSurfaceCreated();
        }
    }
}
