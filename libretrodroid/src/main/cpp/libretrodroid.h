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

#ifndef LIBRETRODROID_LIBRETRODROID_H
#define LIBRETRODROID_LIBRETRODROID_H

#include <jni.h>

#include <EGL/egl.h>

#include <string>
#include <vector>
#include <unordered_set>
#include <mutex>

#include "log.h"
#include "core.h"
#include "audio.h"
#include "video.h"
#include "renderers/renderer.h"
#include "fpssync.h"
#include "input.h"
#include "rumble.h"
#include "shadermanager.h"
#include "javautils.h"
#include "errorcodes.h"
#include "environment.h"
#include "renderers/es3/framebufferrenderer.h"
#include "renderers/es2/imagerendereres2.h"
#include "renderers/es3/imagerendereres3.h"

extern "C" {
#include "utils.h"
#include "libretro/libretro.h"
}

LibretroDroid::Core* core = nullptr;
LibretroDroid::Audio* audio = nullptr;
LibretroDroid::Video* video = nullptr;
LibretroDroid::FPSSync* fpsSync = nullptr;
LibretroDroid::Input* input = nullptr;
LibretroDroid::Rumble* rumble = nullptr;

unsigned int frameSpeed = 1;
bool audioEnabled = true;

std::mutex retroStateMutex;

auto fragmentShaderType = LibretroDroid::ShaderManager::Type::SHADER_DEFAULT;
float screenRefreshRate = 60.0;
int openglESVersion = 2;

uintptr_t callback_get_current_framebuffer();

void callback_hw_video_refresh(const void *data, unsigned width, unsigned height, size_t pitch);

void callback_audio_sample(int16_t left, int16_t right);

size_t callback_set_audio_sample_batch(const int16_t *data, size_t frames);

void callback_retro_set_input_poll();

int16_t callback_set_input_state(unsigned port, unsigned device, unsigned index, unsigned id);

void updateAudioSampleRateMultiplier();

void handlePostStepTasks(JNIEnv* env, jclass obj, jobject glRetroView);

float retrieveGameSpecificAspectRatio();

extern "C" {

JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_reset(JNIEnv* env, jclass obj);
JNIEXPORT jbyteArray JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_serializeState(JNIEnv* env, jclass obj);
JNIEXPORT jboolean JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_unserializeState(JNIEnv* env, jclass obj, jbyteArray data);
JNIEXPORT jbyteArray JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_serializeSRAM(JNIEnv* env, jclass obj);
JNIEXPORT jboolean JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_unserializeSRAM(JNIEnv* env, jclass obj, jbyteArray data);
JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_onSurfaceCreated(JNIEnv* env, jclass obj);
JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_onSurfaceChanged(JNIEnv* env, jclass obj, jint width, jint height);
JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_pause(JNIEnv* env, jclass obj);
JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_resume(JNIEnv* env, jclass obj);
JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_step(JNIEnv* env, jclass obj, jobject glRetroView);
JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_create(JNIEnv* env, jclass obj, jint GLESVersion, jstring soFilePath, jstring systemDir, jstring savesDir, jobjectArray variables, jint shaderType, jfloat refreshRate, jstring language);
JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_loadGameFromPath(JNIEnv* env, jclass obj, jstring gameFilePath);
JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_loadGameFromBytes(JNIEnv* env, jclass obj, jbyteArray gameFileBytes);
JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_destroy(JNIEnv* env, jclass obj);
JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_onKeyEvent(JNIEnv* env, jclass obj, jint port, jint action, jint keyCode);
JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_onMotionEvent(JNIEnv* env, jclass obj, jint port, jint source, jfloat xAxis, jfloat yAxis);
JNIEXPORT jfloat JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_getAspectRatio(JNIEnv* env, jclass obj);
JNIEXPORT jobjectArray JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_getVariables(JNIEnv* env, jclass obj);
JNIEXPORT jobjectArray JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_getControllers(JNIEnv* env, jclass obj);
JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_setControllerType(JNIEnv* env, jclass obj, jint port, jint type);
JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_updateVariable(JNIEnv* env, jclass obj, jobject variable);
JNIEXPORT jint JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_availableDisks(JNIEnv* env, jclass obj);
JNIEXPORT jint JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_currentDisk(JNIEnv* env, jclass obj);
JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_changeDisk(JNIEnv* env, jclass obj, jint index);
JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_setRumbleEnabled(JNIEnv* env, jclass obj, jboolean enabled);
JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_setFrameSpeed(JNIEnv* env, jclass obj, jint speed);
JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_setAudioEnabled(JNIEnv* env, jclass obj, jboolean enabled);

}

#endif //LIBRETRODROID_LIBRETRODROID_H
