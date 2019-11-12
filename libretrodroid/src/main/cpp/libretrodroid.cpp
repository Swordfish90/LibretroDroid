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

// OpenGL ES 2.0 code

#include <jni.h>

#include <EGL/egl.h>

#include <string>
#include <vector>
#include <cmath>
#include <unordered_set>

#include "log.h"
#include "core.h"
#include "audio.h"
#include "video.h"
#include "renderer.h"
#include "fpssync.h"
#include "input.h"

extern "C" {
#include "utils.h"
#include "libretro/libretro.h"
}

LibretroDroid::Core* core = nullptr;
LibretroDroid::Audio* audio = nullptr;
LibretroDroid::Video* video = nullptr;
LibretroDroid::FPSSync* fpsSync = nullptr;
LibretroDroid::Input* input = nullptr;
std::mutex retroStateMutex;

void callback_retro_log(enum retro_log_level level, const char *fmt, ...) {
    va_list argptr;
    va_start(argptr, fmt);

    if (level < RETRO_LOG_DEBUG) {
        LOGD(fmt, argptr);
    } else if (level < RETRO_LOG_ERROR) {
        LOGI(fmt, argptr);
    } else {
        LOGE(fmt, argptr);
    }

    va_end(argptr);
}

void callback_hw_video_refresh(const void *data, unsigned width, unsigned height, size_t pitch) {
    LOGI("hw video refresh callback called %i %i", width, height);
    video->onNewFrame(data, width, height, pitch);
}

std::vector<struct Variable> variables;

struct Variable {
public:
    std::string key;
    std::string value;
};

retro_hw_context_reset_t hw_context_reset = nullptr;
retro_hw_context_reset_t hw_context_destroy = nullptr;

bool environment_handle_set_variables(const struct retro_variable* received) {
    variables.clear();

    unsigned count = 0;
    while (received[count].key != nullptr) {
        LOGI("Received variable %s: %s", received[count].key, received[count].value);

        std::string currentKey(received[count].key);
        std::string currentValue(received[count].value);

        auto firstValueStart = currentValue.find(';') + 2;
        auto firstValueEnd = currentValue.find('|', firstValueStart);

        currentValue = currentValue.substr(firstValueStart, firstValueEnd - firstValueStart);

        auto variable = Variable { currentKey, currentValue };
        variables.push_back(variable);

        LOGI("Assigning variable %s: %s", variable.key.c_str(), variable.value.c_str());

        count++;
    }

    return true;
}

bool environment_handle_get_variable(struct retro_variable* requested) {
    LOGI("%s", requested->key);
    for (auto& variable : variables) {
        if (variable.key == requested->key) {
            requested->value = variable.value.c_str();
            return true;
        }
    }
    return false;
}

bool useHWAcceleration = false;
bool useDepth = false;
bool useStencil = false;
bool bottomLeftOrigin = false;

bool environment_handle_set_hw_render(struct retro_hw_render_callback* hw_render_callback) {
    useHWAcceleration = true;
    useDepth = hw_render_callback->depth;
    useStencil = hw_render_callback->stencil;
    bottomLeftOrigin = hw_render_callback->bottom_left_origin;

    hw_context_destroy = hw_render_callback->context_destroy;
    hw_context_reset = hw_render_callback->context_reset;

    hw_render_callback->get_current_framebuffer = []() -> uintptr_t {
        return video->getCurrentFramebuffer();
    };

    hw_render_callback->get_proc_address = &eglGetProcAddress;

    return true;
}

bool callback_environment(unsigned cmd, void *data) {
    switch (cmd) {
        case RETRO_ENVIRONMENT_GET_CAN_DUPE:
            *((bool*) data) = true;
            return true;

        case RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY:
            *(const char**) data = "/storage/emulated/0/test-system";
            return true;

        case RETRO_ENVIRONMENT_SET_PIXEL_FORMAT:
            LOGI("Called SET_PIXEL_FORMAT");
            return true;

        case RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS:
            LOGI("Callsed SET_INPUT_DESCRIPTORS");
            return false;

        case RETRO_ENVIRONMENT_GET_VARIABLE:
            LOGI("Called RETRO_ENVIRONMENT_GET_VARIABLE");
            return environment_handle_get_variable(static_cast<struct retro_variable*>(data));

        case RETRO_ENVIRONMENT_SET_VARIABLES:
            LOGI("Called RETRO_ENVIRONMENT_SET_VARIABLES");
            return environment_handle_set_variables(static_cast<const struct retro_variable*>(data));

        case RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE:
            LOGI("Called RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE");
            return false;

        case RETRO_ENVIRONMENT_SET_HW_RENDER:
            LOGI("Called RETRO_ENVIRONMENT_SET_HW_RENDER");
            return environment_handle_set_hw_render(static_cast<struct retro_hw_render_callback*>(data));

        case RETRO_ENVIRONMENT_GET_RUMBLE_INTERFACE:
            LOGI("Called RETRO_ENVIRONMENT_GET_RUMBLE_INTERFACE");
            return false;

        case RETRO_ENVIRONMENT_GET_LOG_INTERFACE:
            LOGI("Called RETRO_ENVIRONMENT_GET_LOG_INTERFACE");
            ((struct retro_log_callback*) data)->log = &callback_retro_log;
            return true;

        case RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY:
            LOGI("Called RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY");
            *(const char**) data = "/storage/emulated/0/test-saves";
            return false;

        case RETRO_ENVIRONMENT_GET_PERF_INTERFACE:
            LOGI("Called RETRO_ENVIRONMENT_GET_PERF_INTERFACE");
            return false;

        case RETRO_ENVIRONMENT_SET_GEOMETRY:
            LOGI("Called RETRO_ENVIRONMENT_SET_SYSTEM_AV_INFO");
            return false;

        case RETRO_ENVIRONMENT_SET_SYSTEM_AV_INFO:
            LOGI("Called RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE");
            return false;
    }

    LOGI("callback environment has been called: %u", cmd);
    return false;
}

void callback_audio_sample(int16_t left, int16_t right) {
    LOGE("callback audio sample (left, right) has been called");
}

size_t callback_set_audio_sample_batch(const int16_t *data, size_t frames) {
    if (audio != nullptr) {
        audio->write(data, frames);
    }
    return frames;
}

void callback_retro_set_input_poll() {
    // Do nothing in here...
}

int16_t callback_set_input_state(unsigned port, unsigned device, unsigned index, unsigned id) {
    LOGD("callback_set_input_state called. Retrieving user input. %u %u %u %u", port, device, index, id);
    return input->getInputState(port, device, index, id);
}

// INITIALIZATION

extern "C" {
    JNIEXPORT jbyteArray JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_serialize(JNIEnv * env, jobject obj);
    JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_unserialize(JNIEnv * env, jobject obj, jbyteArray data);
    JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_onSurfaceCreated(JNIEnv * env, jobject obj);
    JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_onSurfaceChanged(JNIEnv * env, jobject obj, jint width, jint height);
    JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_pause(JNIEnv * env, jobject obj);
    JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_resume(JNIEnv * env, jobject obj);
    JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_step(JNIEnv * env, jobject obj);
    JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_create(JNIEnv * env, jobject obj, jstring soFilePath, jstring gameFilePath);
    JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_destroy(JNIEnv * env, jobject obj);
    JNIEXPORT jboolean JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_onKeyEvent(JNIEnv * env, jobject obj, jint action, jint keyCode);
    JNIEXPORT jboolean JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_onMotionEvent(JNIEnv * env, jobject obj, jint source, jfloat xAxis, jfloat yAxis);
};

JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_unserialize(JNIEnv * env, jobject obj, jbyteArray data) {
    jboolean isCopy = JNI_FALSE;
    jbyte* cData = env->GetByteArrayElements(data, &isCopy);
    jsize stateSize = env->GetArrayLength(data);

    retroStateMutex.lock();
    core->retro_unserialize(cData, (size_t) stateSize);
    retroStateMutex.unlock();

    env->ReleaseByteArrayElements(data, cData, JNI_ABORT);
}

JNIEXPORT jbyteArray JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_serialize(JNIEnv * env, jobject obj) {
    size_t size = core->retro_serialize_size();
    jbyte* state = new jbyte[size];

    retroStateMutex.lock();
    core->retro_serialize(state, size);
    retroStateMutex.unlock();

    jbyteArray result = env->NewByteArray(size);
    env->SetByteArrayRegion (result, 0, size, state);

    return result;
}

JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_onSurfaceChanged(JNIEnv * env, jobject obj, jint width, jint height) {
    LOGD("Performing LibretroDroid onSurfaceChanged");
    video->updateScreenSize(width, height);
}

JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_onSurfaceCreated(JNIEnv * env, jobject obj) {
    LOGD("Performing LibretroDroid onSurfaceCreated");

    struct retro_system_av_info system_av_info;
    core->retro_get_system_av_info(&system_av_info);

    if (video != nullptr) {
        free(video);
        video = nullptr;
    }

    LibretroDroid::Renderer* renderer;
    if (useHWAcceleration) {
        renderer = new LibretroDroid::FramebufferRenderer(
                system_av_info.geometry.base_width,
                system_av_info.geometry.base_height,
                useDepth,
                useStencil
        );
    } else {
        renderer = new LibretroDroid::ImageRenderer();
    }

    LibretroDroid::Video* newVideo = new LibretroDroid::Video();
    newVideo->initializeGraphics(
            renderer,
            bottomLeftOrigin,
            system_av_info.geometry.aspect_ratio
    );

    video = newVideo;

    if (hw_context_reset != nullptr) {
        hw_context_reset();
    }
}

JNIEXPORT jboolean JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_onMotionEvent(JNIEnv * env, jobject obj, jint source, jfloat xAxis, jfloat yAxis) {
    LOGD("Received motion event: %d %.2f, %.2f", source, xAxis, yAxis);
    bool result = input->onMotionEvent(source, xAxis, yAxis);
    return (jboolean) (result ? JNI_TRUE : JNI_FALSE);
}

JNIEXPORT jboolean JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_onKeyEvent(JNIEnv * env, jobject obj, jint action, jint keyCode) {
    LOGD("Received key event with action (%d) and keycode (%d)", action, keyCode);
    bool result = input->onKeyEvent(action, keyCode);
    return (jboolean) (result ? JNI_TRUE : JNI_FALSE);
}

JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_create(JNIEnv * env, jobject obj, jstring soFilePath, jstring gameFilePath) {
    LOGD("Performing LibretroDroid create");
    const char* corePath = env->GetStringUTFChars(soFilePath, nullptr);
    const char* gamePath = env->GetStringUTFChars(gameFilePath, nullptr);

    core = new LibretroDroid::Core(corePath);

    core->retro_set_video_refresh(&callback_hw_video_refresh);
    core->retro_set_environment(&callback_environment);
    core->retro_set_audio_sample(&callback_audio_sample);
    core->retro_set_audio_sample_batch(&callback_set_audio_sample_batch);
    core->retro_set_input_poll(&callback_retro_set_input_poll);
    core->retro_set_input_state(&callback_set_input_state);

    core->retro_init();

    struct retro_system_info system_info;
    core->retro_get_system_info(&system_info);

    struct retro_game_info game_info;
    if (system_info.need_fullpath) {
        game_info.path = gamePath;
        game_info.data = nullptr;
        game_info.size = 0;
    } else {
        struct read_file_result file = read_file_as_bytes(gamePath);
        game_info.data = file.data;
        game_info.size = file.size;
    }

    bool result = core->retro_load_game(&game_info);
    if (!result) {
        LOGI("Cannot load game. Leaving.");
        exit(1);
    }

    env->ReleaseStringUTFChars(soFilePath, corePath);
    env->ReleaseStringUTFChars(gameFilePath, gamePath);
}

JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_destroy(JNIEnv * env, jobject obj) {
    LOGD("Performing LibretroDroid destroy");

    if (hw_context_destroy != nullptr) {
        hw_context_destroy();
    }

    core->retro_deinit();

    free(video);
    video = nullptr;

    free(core);
    core = nullptr;

    hw_context_destroy = nullptr;
    hw_context_reset = nullptr;
}

JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_resume(JNIEnv * env, jobject obj) {
    LOGD("Performing LibretroDroid resume");

    input = new LibretroDroid::Input();

    struct retro_system_av_info system_av_info;
    core->retro_get_system_av_info(&system_av_info);

    audio = new LibretroDroid::Audio(std::lround(system_av_info.timing.sample_rate));
    audio->start();

    fpsSync = new LibretroDroid::FPSSync(system_av_info.timing.fps);
    fpsSync->start();
}

JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_pause(JNIEnv * env, jobject obj) {
    LOGD("Performing LibretroDroid pause");

    free(input);

    audio->stop();
    free(audio);

    free(fpsSync);
}

JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_step(JNIEnv * env, jobject obj)
{
    LOGD("Stepping into retro_run()");

    retroStateMutex.lock();
    core->retro_run();
    retroStateMutex.unlock();

    if (video != nullptr) {
        video->renderFrame();
    }

    fpsSync->sync();
}


