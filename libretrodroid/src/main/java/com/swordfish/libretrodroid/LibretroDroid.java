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

import java.util.List;

class LibretroDroid {

    static {
        System.loadLibrary("libretrodroid");
    }

    public static final int MOTION_SOURCE_DPAD = 0;
    public static final int MOTION_SOURCE_ANALOG_LEFT = 1;
    public static final int MOTION_SOURCE_ANALOG_RIGHT = 2;
    public static final int MOTION_SOURCE_POINTER = 3;

    public static final int SHADER_DEFAULT = 0;
    public static final int SHADER_CRT = 1;
    public static final int SHADER_LCD = 2;
    public static final int SHADER_SHARP = 3;

    public static final int ERROR_LOAD_LIBRARY = 0;
    public static final int ERROR_LOAD_GAME = 1;
    public static final int ERROR_GL_NOT_COMPATIBLE = 2;
    public static final int ERROR_SERIALIZATION = 3;
    public static final int ERROR_GENERIC = 4;

    public static native void create(
        int GLESVersion,
        String coreFilePath,
        String systemDir,
        String savesDir,
        Variable[] variables,
        int shaderType,
        float refreshRate,
        boolean preferLowLatencyAudio,
        boolean enableVirtualFileSystem,
        String language
    );

    public static native void loadGameFromPath(String gameFilePath);
    public static native void loadGameFromBytes(byte[] gameFileBytes);
    public static native void loadGameFromVirtualFiles(List<DetachedVirtualFile> virtualFiles);
    public static native void resume();

    public static native void onSurfaceCreated();
    public static native void onSurfaceChanged(int width, int height);

    public static native void pause();
    public static native void destroy();

    public static native void step(GLRetroView retroView);

    public static native void reset();

    public static native void setRumbleEnabled(boolean enabled);
    public static native void setFrameSpeed(int speed);
    public static native void setAudioEnabled(boolean enabled);

    public static native byte[] serializeState();
    public static native boolean unserializeState(byte[] state);

    public static native byte[] serializeSRAM();
    public static native boolean unserializeSRAM(byte[] sram);

    public static native void updateVariable(Variable variable);
    public static native Variable[] getVariables();

    public static native int availableDisks();
    public static native int currentDisk();
    public static native void changeDisk(int index);

    /** Send motion events. Analog events in range [-1, +1] and touch events in range [0,1] */
    public static native void onMotionEvent(int port, int motionSource, float xAxis, float yAxis);

    public static native void onKeyEvent(int port, int action, int keyCode);

    public static native float getAspectRatio();

    public static native Controller[][] getControllers();
    public static native void setControllerType(int port, int type);
}
