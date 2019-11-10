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
    private static String TAG = "GLRetroView";
    private static final boolean DEBUG = false;

    public GLRetroView(Context context) {
        super(context);
        init();
    }

    public GLRetroView(Context context, boolean translucent, int depth, int stencil) {
        super(context);
        init();
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
        setEGLContextFactory(new ContextFactory());

        /* We need to choose an EGLConfig that matches the format of
         * our surface exactly. This is going to be done in our
         * custom config chooser. See ConfigChooser class definition
         * below.
         */
        /*setEGLConfigChooser( translucent ?
                             new ConfigChooser(8, 8, 8, 8, depth, stencil) :
                             new ConfigChooser(5, 6, 5, 0, depth, stencil) );*/

        /* Set the renderer responsible for frame rendering */

        //LibretroDroid.create("gambatte_libretro_android.so", "/storage/emulated/0/Roms Test/gb/Super Mario Land.gb");
        //LibretroDroid.create("gambatte_libretro_android.so", "/storage/emulated/0/Roms Test/gb/Pokemon Blue Version.gb");
        //LibretroDroid.create("gambatte_libretro_android.so", "/storage/emulated/0/Roms Test/gb/Super Mario Land 2 - 6 Golden Coins.gb");
        //LibretroDroid.create("gambatte_libretro_android.so", "/storage/emulated/0/Roms Test/gb/Tetris.gb");
        LibretroDroid.create("mupen64plus_next_gles3_libretro_android.so", "/storage/emulated/0/Roms Test/n64/Super Mario 64/Super Mario 64.n64");
        //LibretroDroid.create("mupen64plus_next_gles3_libretro_android.so", "/storage/emulated/0/Roms Test/n64/Legend of Zelda, The - Ocarina of Time - Master Quest/Legend of Zelda, The - Ocarina of Time - Master Quest.z64");

        setRenderer(new Renderer());
    }

    private static class ContextFactory implements GLSurfaceView.EGLContextFactory {
        private static int EGL_CONTEXT_CLIENT_VERSION = 0x3098;
        public EGLContext createContext(EGL10 egl, EGLDisplay display, EGLConfig eglConfig) {
            Log.w(TAG, "creating OpenGL ES 3.0 context");
            checkEglError("Before eglCreateContext", egl);
            int[] attrib_list = { EGL_CONTEXT_CLIENT_VERSION, 3, EGL10.EGL_NONE };
            EGLContext context = egl.eglCreateContext(display, eglConfig, EGL10.EGL_NO_CONTEXT, attrib_list);
            checkEglError("After eglCreateContext", egl);
            return context;
        }

        public void destroyContext(EGL10 egl, EGLDisplay display, EGLContext context) {
            egl.eglDestroyContext(display, context);
        }
    }

    private static void checkEglError(String prompt, EGL10 egl) {
        int error;
        while ((error = egl.eglGetError()) != EGL10.EGL_SUCCESS) {
            Log.e(TAG, String.format("%s: EGL error: 0x%x", prompt, error));
        }
    }

    private static class Renderer implements GLSurfaceView.Renderer {
        public void onDrawFrame(GL10 gl) {
            LibretroDroid.step();
        }

        public void onSurfaceChanged(GL10 gl, int width, int height) {
            LibretroDroid.setupGraphics(width, height);
        }

        public void onSurfaceCreated(GL10 gl, EGLConfig config) {
            // Do nothing.
        }
    }
}
