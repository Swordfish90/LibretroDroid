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

#define MODULE_NAME_CORE "Libretro Core"

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
#include "shadermanager.h"
#include "javautils.h"

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
auto fragmentShaderType = LibretroDroid::ShaderManager::Type::SHADER_DEFAULT;
const char* savesDirectory = nullptr;
const char* systemDirectory = nullptr;

void callback_retro_log(enum retro_log_level level, const char *fmt, ...) {
    va_list argptr;
    va_start(argptr, fmt);

    switch (level) {
        case RETRO_LOG_DEBUG:
            __android_log_vprint(ANDROID_LOG_DEBUG, MODULE_NAME_CORE, fmt, argptr);
            break;
        case RETRO_LOG_INFO:
            __android_log_vprint(ANDROID_LOG_INFO, MODULE_NAME_CORE, fmt, argptr);
            break;
        case RETRO_LOG_WARN:
            __android_log_vprint(ANDROID_LOG_WARN, MODULE_NAME_CORE, fmt, argptr);
            break;
        case RETRO_LOG_ERROR:
            __android_log_vprint(ANDROID_LOG_ERROR, MODULE_NAME_CORE, fmt, argptr);
            break;
        default:
            // Log nothing in here.
            break;
    }
}

void callback_hw_video_refresh(const void *data, unsigned width, unsigned height, size_t pitch) {
    LOGD("hw video refresh callback called %i %i", width, height);
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
        LOGD("Received variable %s: %s", received[count].key, received[count].value);

        std::string currentKey(received[count].key);
        std::string currentValue(received[count].value);

        auto firstValueStart = currentValue.find(';') + 2;
        auto firstValueEnd = currentValue.find('|', firstValueStart);

        currentValue = currentValue.substr(firstValueStart, firstValueEnd - firstValueStart);

        auto variable = Variable { currentKey, currentValue };
        variables.push_back(variable);

        LOGD("Assigning variable %s: %s", variable.key.c_str(), variable.value.c_str());

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

        case RETRO_ENVIRONMENT_SET_PIXEL_FORMAT:
            LOGD("Called SET_PIXEL_FORMAT");
            return true;

        case RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS:
            LOGD("Called SET_INPUT_DESCRIPTORS");
            return false;

        case RETRO_ENVIRONMENT_GET_VARIABLE:
            LOGD("Called RETRO_ENVIRONMENT_GET_VARIABLE");
            return environment_handle_get_variable(static_cast<struct retro_variable*>(data));

        case RETRO_ENVIRONMENT_SET_VARIABLES:
            LOGD("Called RETRO_ENVIRONMENT_SET_VARIABLES");
            return environment_handle_set_variables(static_cast<const struct retro_variable*>(data));

        case RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE:
            LOGD("Called RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE");
            return false;

        case RETRO_ENVIRONMENT_SET_HW_RENDER:
            LOGD("Called RETRO_ENVIRONMENT_SET_HW_RENDER");
            return environment_handle_set_hw_render(static_cast<struct retro_hw_render_callback*>(data));

        case RETRO_ENVIRONMENT_GET_RUMBLE_INTERFACE:
            LOGD("Called RETRO_ENVIRONMENT_GET_RUMBLE_INTERFACE");
            return false;

        case RETRO_ENVIRONMENT_GET_LOG_INTERFACE:
            LOGD("Called RETRO_ENVIRONMENT_GET_LOG_INTERFACE");
            ((struct retro_log_callback*) data)->log = &callback_retro_log;
            return true;

        case RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY:
            LOGD("Called RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY");
            *(const char**) data = savesDirectory;
            return savesDirectory != nullptr;

        case RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY:
            *(const char**) data = systemDirectory;
            return systemDirectory != nullptr;

        case RETRO_ENVIRONMENT_GET_PERF_INTERFACE:
            LOGD("Called RETRO_ENVIRONMENT_GET_PERF_INTERFACE");
            return false;

        case RETRO_ENVIRONMENT_SET_GEOMETRY:
            LOGD("Called RETRO_ENVIRONMENT_SET_SYSTEM_AV_INFO");
            return false;

        case RETRO_ENVIRONMENT_SET_SYSTEM_AV_INFO:
            LOGD("Called RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE");
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
    if (input != nullptr) {
        return input->getInputState(port, device, index, id);
    }
    return 0;
}

// INITIALIZATION

extern "C" {
    JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_reset(JNIEnv * env, jobject obj);
    JNIEXPORT jbyteArray JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_serialize(JNIEnv * env, jobject obj);
    JNIEXPORT jboolean JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_unserialize(JNIEnv * env, jobject obj, jbyteArray data);
    JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_onSurfaceCreated(JNIEnv * env, jobject obj);
    JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_onSurfaceChanged(JNIEnv * env, jobject obj, jint width, jint height);
    JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_pause(JNIEnv * env, jobject obj);
    JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_resume(JNIEnv * env, jobject obj);
    JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_step(JNIEnv * env, jobject obj);
    JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_create(JNIEnv * env, jobject obj, jstring soFilePath, jstring gameFilePath, jstring systemDir, jstring savesDir, jint shaderType);
    JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_destroy(JNIEnv * env, jobject obj);
    JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_onKeyEvent(JNIEnv * env, jobject obj, jint port, jint action, jint keyCode);
    JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_onMotionEvent(JNIEnv * env, jobject obj, jint port, jint source, jfloat xAxis, jfloat yAxis);
};

JNIEXPORT jboolean JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_unserialize(JNIEnv * env, jobject obj, jbyteArray data) {
    try {
        jboolean isCopy = JNI_FALSE;
        jbyte* cData = env->GetByteArrayElements(data, &isCopy);
        jsize stateSize = env->GetArrayLength(data);

        retroStateMutex.lock();
        bool result = core->retro_unserialize(cData, (size_t) stateSize);
        retroStateMutex.unlock();
        env->ReleaseByteArrayElements(data, cData, JNI_ABORT);

        return result ? JNI_TRUE : JNI_FALSE;

    } catch (std::exception& exception) {
        LibretroDroid::JavaUtils::throwRuntimeException(env, exception.what());
        return JNI_FALSE;
    }
}

JNIEXPORT jbyteArray JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_serialize(JNIEnv * env, jobject obj) {

    try {
        retroStateMutex.lock();

        size_t size = core->retro_serialize_size();
        jbyte* state = new jbyte[size];

        core->retro_serialize(state, size);

        retroStateMutex.unlock();

        jbyteArray result = env->NewByteArray(size);
        env->SetByteArrayRegion (result, 0, size, state);

        return result;

    } catch (std::exception& exception) {
        LibretroDroid::JavaUtils::throwRuntimeException(env, exception.what());
    }
}

JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_reset(JNIEnv * env, jobject obj) {
    try {
        retroStateMutex.lock();
        core->retro_reset();
        retroStateMutex.unlock();

    } catch (std::exception& exception) {
        LibretroDroid::JavaUtils::throwRuntimeException(env, exception.what());
    }
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
        delete video;
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
            LibretroDroid::ShaderManager::getShader(fragmentShaderType),
            bottomLeftOrigin,
            system_av_info.geometry.aspect_ratio
    );

    video = newVideo;

    if (hw_context_reset != nullptr) {
        hw_context_reset();
    }
}

JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_onMotionEvent(JNIEnv * env, jobject obj, jint port, jint source, jfloat xAxis, jfloat yAxis) {
    LOGD("Received motion event: %d %.2f, %.2f", source, xAxis, yAxis);
    if (input != nullptr) {
        input->onMotionEvent(port, source, xAxis, yAxis);
    }
}

JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_onKeyEvent(JNIEnv * env, jobject obj, jint port, jint action, jint keyCode) {
    LOGD("Received key event with action (%d) and keycode (%d)", action, keyCode);
    if (input != nullptr) {
        input->onKeyEvent(port, action, keyCode);
    }
}

JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_create(
    JNIEnv * env,
    jobject obj,
    jstring soFilePath,
    jstring gameFilePath,
    jstring systemDir,
    jstring savesDir,
    jint shaderType
) {
    LOGD("Performing LibretroDroid create");
    const char* corePath = env->GetStringUTFChars(soFilePath, nullptr);
    const char* gamePath = env->GetStringUTFChars(gameFilePath, nullptr);

    try {
        systemDirectory = env->GetStringUTFChars(systemDir, nullptr);
        savesDirectory = env->GetStringUTFChars(savesDir, nullptr);

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
        game_info.path = gamePath;
        game_info.meta = nullptr;

        if (system_info.need_fullpath) {
            game_info.data = nullptr;
            game_info.size = 0;
        } else {
            struct LibretroDroid::Utils::ReadResult file = LibretroDroid::Utils::readFileAsBytes(gamePath);
            game_info.data = file.data;
            game_info.size = file.size;
        }

        bool result = core->retro_load_game(&game_info);
        if (!result) {
            LOGE("Cannot load game. Leaving.");
            throw std::runtime_error("Cannot load game");
        }

        env->ReleaseStringUTFChars(soFilePath, corePath);
        env->ReleaseStringUTFChars(gameFilePath, gamePath);

        fragmentShaderType = LibretroDroid::ShaderManager::Type(shaderType);

    } catch (std::exception& exception) {
        LibretroDroid::JavaUtils::throwRuntimeException(env, exception.what());
    }
}

JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_destroy(JNIEnv * env, jobject obj) {
    LOGD("Performing LibretroDroid destroy");

    try {
        if (hw_context_destroy != nullptr) {
            hw_context_destroy();
        }

        core->retro_deinit();

        delete video;
        video = nullptr;

        delete core;
        core = nullptr;

        hw_context_destroy = nullptr;
        hw_context_reset = nullptr;

    } catch (std::exception& exception) {
        LibretroDroid::JavaUtils::throwRuntimeException(env, exception.what());
    }
}

JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_resume(JNIEnv * env, jobject obj) {
    LOGD("Performing LibretroDroid resume");

    try {
        input = new LibretroDroid::Input();

        struct retro_system_av_info system_av_info;
        core->retro_get_system_av_info(&system_av_info);

        audio = new LibretroDroid::Audio(std::lround(system_av_info.timing.sample_rate));
        audio->start();

        fpsSync = new LibretroDroid::FPSSync(system_av_info.timing.fps);
        fpsSync->start();

    } catch (std::exception& exception) {
        LibretroDroid::JavaUtils::throwRuntimeException(env, exception.what());
    }
}

JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_pause(JNIEnv * env, jobject obj) {
    LOGD("Performing LibretroDroid pause");

    try {
        delete input;
        input = nullptr;

        audio->stop();
        delete audio;
        audio = nullptr;

        delete fpsSync;
        fpsSync = nullptr;

    } catch (std::exception& exception) {
        LibretroDroid::JavaUtils::throwRuntimeException(env, exception.what());
    }
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

    if (fpsSync != nullptr) {
        fpsSync->sync();
    }
}


