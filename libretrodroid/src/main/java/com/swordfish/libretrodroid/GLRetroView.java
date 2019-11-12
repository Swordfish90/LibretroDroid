/*
 * Copyright (C) 2009 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.swordfish.libretrodroid;
/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


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

/**
 * A simple GLSurfaceView sub-class that demonstrate how to perform
 * OpenGL ES 2.0 rendering into a GL Surface. Note the following important
 * details:
 *
 * - The class must use a custom context factory to enable 2.0 rendering.
 *   See ContextFactory class definition below.
 *
 * - The class must use a custom EGLConfigChooser to be able to select
 *   an EGLConfig that supports 2.0. This is done by providing a config
 *   specification to eglChooseConfig() that has the attribute
 *   EGL10.ELG_RENDERABLE_TYPE containing the EGL_OPENGL_ES2_BIT flag
 *   set. See ConfigChooser class definition below.
 *
 * - The class must select the surface's format, then choose an EGLConfig
 *   that matches it exactly (with regards to red/green/blue/alpha channels
 *   bit depths). Failure to do so would result in an EGL_BAD_MATCH error.
 */
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
