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
#include "environment.cpp"
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

bool fastForwardEnabled = false;
bool audioEnabled = true;

std::mutex retroStateMutex;

auto fragmentShaderType = LibretroDroid::ShaderManager::Type::SHADER_DEFAULT;
float screenRefreshRate = 60.0;
int openglESVersion = 2;

uintptr_t callback_get_current_framebuffer() {
    if (video != nullptr) {
        return video->getCurrentFramebuffer();
    }
    return 0;
}

void callback_hw_video_refresh(const void *data, unsigned width, unsigned height, size_t pitch) {
    LOGD("hw video refresh callback called %i %i", width, height);
    video->onNewFrame(data, width, height, pitch);
}

void callback_audio_sample(int16_t left, int16_t right) {
    LOGE("callback audio sample (left, right) has been called");
}

size_t callback_set_audio_sample_batch(const int16_t *data, size_t frames) {
    if (audio != nullptr && audioEnabled) {
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

void updateAudioSampleRateMultiplier() {
    if (audio != nullptr) {
        audio->setSampleRateMultiplier(fastForwardEnabled ? 2.0 : 1.0);
    }
}

// INITIALIZATION

extern "C" {
    JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_reset(JNIEnv * env, jobject obj);
    JNIEXPORT jbyteArray JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_serializeState(JNIEnv * env, jobject obj);
    JNIEXPORT jboolean JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_unserializeState(JNIEnv * env, jobject obj, jbyteArray data);
    JNIEXPORT jbyteArray JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_serializeSRAM(JNIEnv * env, jobject obj);
    JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_unserializeSRAM(JNIEnv * env, jobject obj, jbyteArray data);
    JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_onSurfaceCreated(JNIEnv * env, jobject obj);
    JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_onSurfaceChanged(JNIEnv * env, jobject obj, jint width, jint height);
    JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_pause(JNIEnv * env, jobject obj);
    JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_resume(JNIEnv * env, jobject obj);
    JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_step(JNIEnv * env, jobject obj, jobject glRetroView);
    JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_create(JNIEnv * env, jobject obj, jint GLESVersion, jstring soFilePath, jstring systemDir, jstring savesDir, jobjectArray variables, jint shaderType, jfloat refreshRate, jstring language);
    JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_loadGameFromPath(JNIEnv * env, jobject obj, jstring gameFilePath);
    JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_loadGameFromBytes(JNIEnv * env, jobject obj, jbyteArray gameFileBytes);
    JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_destroy(JNIEnv * env, jobject obj);
    JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_onKeyEvent(JNIEnv * env, jobject obj, jint port, jint action, jint keyCode);
    JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_onMotionEvent(JNIEnv * env, jobject obj, jint port, jint source, jfloat xAxis, jfloat yAxis);
    JNIEXPORT jfloat JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_getAspectRatio(JNIEnv * env, jobject obj);
    JNIEXPORT jobjectArray JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_getVariables(JNIEnv * env, jobject obj);
    JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_updateVariable(JNIEnv * env, jobject obj, jobject variable);
    JNIEXPORT jint JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_availableDisks(JNIEnv * env, jobject obj);
    JNIEXPORT jint JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_currentDisk(JNIEnv * env, jobject obj);
    JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_changeDisk(JNIEnv * env, jobject obj, jint index);
    JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_setRumbleEnabled(JNIEnv * env, jobject obj, jboolean enabled);
    JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_setFastForwardEnabled(JNIEnv * env, jobject obj, jboolean enabled);
    JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_setAudioEnabled(JNIEnv * env, jobject obj, jboolean enabled);
};

JNIEXPORT jint JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_availableDisks(JNIEnv * env, jobject obj) {
    std::lock_guard<std::mutex> lock(retroStateMutex);
    return Environment::retro_disk_control_callback != nullptr
        ? Environment::retro_disk_control_callback->get_num_images()
        : 0;
}

JNIEXPORT jint JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_currentDisk(JNIEnv * env, jobject obj) {
    std::lock_guard<std::mutex> lock(retroStateMutex);
    return Environment::retro_disk_control_callback != nullptr
           ? Environment::retro_disk_control_callback->get_image_index()
           : 0;
}

JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_changeDisk(JNIEnv * env, jobject obj, jint index) {
    std::lock_guard<std::mutex> lock(retroStateMutex);
    if (Environment::retro_disk_control_callback == nullptr) {
        LOGE("Cannot swap disk. This platform does not support it.");
        return;
    }

    if (index < 0 || index >= Environment::retro_disk_control_callback->get_num_images()) {
        LOGE("Requested image index is not valid.");
        return;
    }

    if (Environment::retro_disk_control_callback->get_image_index() != index) {
        Environment::retro_disk_control_callback->set_eject_state(true);
        Environment::retro_disk_control_callback->set_image_index((unsigned) index);
        Environment::retro_disk_control_callback->set_eject_state(false);
    }
}

JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_updateVariable(JNIEnv * env, jobject obj, jobject variable) {
    jclass variableClass = env->FindClass("com/swordfish/libretrodroid/Variable");

    jfieldID jKeyField = env->GetFieldID(variableClass, "key", "Ljava/lang/String;");
    jfieldID jValueField = env->GetFieldID(variableClass, "value", "Ljava/lang/String;");

    auto jKeyObject = (jstring) env->GetObjectField(variable, jKeyField);
    auto jValueObject = (jstring) env->GetObjectField(variable, jValueField);

    jboolean isCopy = JNI_TRUE;

    Environment::updateVariable(
        env->GetStringUTFChars(jKeyObject, &isCopy),
        env->GetStringUTFChars(jValueObject, &isCopy)
    );
}

JNIEXPORT jobjectArray JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_getVariables(JNIEnv * env, jobject obj) {
    jclass variableClass = env->FindClass("com/swordfish/libretrodroid/Variable");
    jmethodID variableMethod = env->GetMethodID(variableClass, "<init>", "()V");

    auto variables = Environment::variables;
    jobjectArray result = env->NewObjectArray(variables.size(), variableClass, nullptr);

    for (int i = 0; i < variables.size(); i++) {
        jobject jVariable = env->NewObject(variableClass, variableMethod);

        jfieldID jKeyField = env->GetFieldID(variableClass, "key", "Ljava/lang/String;");
        jfieldID jValueField = env->GetFieldID(variableClass, "value", "Ljava/lang/String;");
        jfieldID jDescriptionField = env->GetFieldID(variableClass, "description", "Ljava/lang/String;");

        env->SetObjectField(jVariable, jKeyField, env->NewStringUTF(variables[i].key.data()));
        env->SetObjectField(jVariable, jValueField, env->NewStringUTF(variables[i].value.data()));
        env->SetObjectField(jVariable, jDescriptionField, env->NewStringUTF(variables[i].description.data()));

        env->SetObjectArrayElement(result, i, jVariable);
    }
    return result;
}

JNIEXPORT jboolean JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_unserializeState(JNIEnv * env, jobject obj, jbyteArray data) {
    std::lock_guard<std::mutex> lock(retroStateMutex);

    try {
        jboolean isCopy = JNI_FALSE;
        jbyte* cData = env->GetByteArrayElements(data, &isCopy);
        jsize stateSize = env->GetArrayLength(data);

        bool result = core->retro_unserialize(cData, (size_t) stateSize);
        env->ReleaseByteArrayElements(data, cData, JNI_ABORT);

        return result ? JNI_TRUE : JNI_FALSE;

    } catch (std::exception& exception) {
        LibretroDroid::JavaUtils::throwRetroException(env, LibretroDroid::ERROR_SERIALIZATION);
        return JNI_FALSE;
    }
}

JNIEXPORT jbyteArray JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_serializeState(JNIEnv * env, jobject obj) {
    std::lock_guard<std::mutex> lock(retroStateMutex);

    try {
        size_t size = core->retro_serialize_size();
        jbyte* state = new jbyte[size];

        core->retro_serialize(state, size);

        jbyteArray result = env->NewByteArray(size);
        env->SetByteArrayRegion (result, 0, size, state);

        return result;

    } catch (std::exception& exception) {
        LibretroDroid::JavaUtils::throwRetroException(env, LibretroDroid::ERROR_SERIALIZATION);
    }
}

JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_unserializeSRAM(JNIEnv * env, jobject obj, jbyteArray data) {
    std::lock_guard<std::mutex> lock(retroStateMutex);

    try {
        jboolean isCopy = JNI_FALSE;
        jbyte* cData = env->GetByteArrayElements(data, &isCopy);
        jsize stateSize = env->GetArrayLength(data);

        size_t sramSize = core->retro_get_memory_size(RETRO_MEMORY_SAVE_RAM);
        void* sramState = core->retro_get_memory_data(RETRO_MEMORY_SAVE_RAM);

        if (sramState == nullptr) {
            LOGE("Cannot load SRAM: nullptr in retro_get_memory_data");
            env->ReleaseByteArrayElements(data, cData, JNI_ABORT);
            return;
        }

        if (sramSize != stateSize) {
            LOGE("Cannot load SRAM: size mismatch");
            env->ReleaseByteArrayElements(data, cData, JNI_ABORT);
            return;
        }

        memcpy(sramState, cData, sramSize);
        env->ReleaseByteArrayElements(data, cData, JNI_ABORT);

    } catch (std::exception& exception) {
        LibretroDroid::JavaUtils::throwRetroException(env, LibretroDroid::ERROR_SERIALIZATION);
    }
}

JNIEXPORT jbyteArray JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_serializeSRAM(JNIEnv * env, jobject obj) {
    std::lock_guard<std::mutex> lock(retroStateMutex);

    try {
        size_t size = core->retro_get_memory_size(RETRO_MEMORY_SAVE_RAM);
        void* sram = core->retro_get_memory_data(RETRO_MEMORY_SAVE_RAM);

        jbyteArray result = env->NewByteArray(size);
        env->SetByteArrayRegion(result, 0, size, (jbyte*) sram);

        return result;

    } catch (std::exception& exception) {
        LibretroDroid::JavaUtils::throwRetroException(env, LibretroDroid::ERROR_SERIALIZATION);
    }
}

JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_reset(JNIEnv * env, jobject obj) {
    std::lock_guard<std::mutex> lock(retroStateMutex);

    try {
        core->retro_reset();
    } catch (std::exception& exception) {
        LibretroDroid::JavaUtils::throwRetroException(env, LibretroDroid::ERROR_GENERIC);
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
    if (Environment::useHWAcceleration) {
        renderer = new LibretroDroid::FramebufferRenderer(
                system_av_info.geometry.base_width,
                system_av_info.geometry.base_height,
                Environment::useDepth,
                Environment::useStencil
        );
    } else {
        if (openglESVersion >= 3) {
            renderer = new LibretroDroid::ImageRendererES3();
        } else {
            renderer = new LibretroDroid::ImageRendererES2();
        }
    }

    auto newVideo = new LibretroDroid::Video();
    newVideo->initializeGraphics(
            renderer,
            LibretroDroid::ShaderManager::getShader(fragmentShaderType),
            Environment::bottomLeftOrigin,
            Environment::screenRotation
    );

    renderer->setPixelFormat(Environment::pixelFormat);

    video = newVideo;

    if (Environment::hw_context_reset != nullptr) {
        Environment::hw_context_reset();
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
    jint GLESVersion,
    jstring soFilePath,
    jstring systemDir,
    jstring savesDir,
    jobjectArray variables,
    jint shaderType,
    jfloat refreshRate,
    jstring language
) {
    LOGD("Performing LibretroDroid create");
    const char* corePath = env->GetStringUTFChars(soFilePath, nullptr);
    const char* deviceLanguage = env->GetStringUTFChars(language, nullptr);

    try {
        Environment::initialize(
            env->GetStringUTFChars(systemDir, nullptr),
            env->GetStringUTFChars(savesDir, nullptr),
            &callback_get_current_framebuffer
        );

        Environment::setLanguage(std::string(deviceLanguage));

        openglESVersion = GLESVersion;
        screenRefreshRate = refreshRate;
        audioEnabled = true;
        fastForwardEnabled = false;

        core = new LibretroDroid::Core(corePath);

        core->retro_set_video_refresh(&callback_hw_video_refresh);
        core->retro_set_environment(&Environment::callback_environment);
        core->retro_set_audio_sample(&callback_audio_sample);
        core->retro_set_audio_sample_batch(&callback_set_audio_sample_batch);
        core->retro_set_input_poll(&callback_retro_set_input_poll);
        core->retro_set_input_state(&callback_set_input_state);

        int size = env->GetArrayLength(variables);
        for (int i = 0; i < size; i++) {
            auto variable = (jobject) env->GetObjectArrayElement(variables, i);
            Java_com_swordfish_libretrodroid_LibretroDroid_updateVariable(env, obj, variable);
        }

        core->retro_init();

        fragmentShaderType = LibretroDroid::ShaderManager::Type(shaderType);

        // HW accelerated cores are only supported on opengles 3.
        if (Environment::useHWAcceleration && openglESVersion < 3) {
            LibretroDroid::JavaUtils::throwRetroException(env, LibretroDroid::ERROR_GL_NOT_COMPATIBLE);
        }

        env->ReleaseStringUTFChars(soFilePath, corePath);
        env->ReleaseStringUTFChars(language, deviceLanguage);

        fragmentShaderType = LibretroDroid::ShaderManager::Type(shaderType);

        rumble = new LibretroDroid::Rumble();

    } catch (std::exception& exception) {
        LibretroDroid::JavaUtils::throwRetroException(env, LibretroDroid::ERROR_LOAD_LIBRARY);
    }
}

JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_loadGameFromPath(
    JNIEnv * env,
    jobject obj,
    jstring gameFilePath
) {
    LOGD("Performing LibretroDroid loadGameFromPath");
    const char* gamePath = env->GetStringUTFChars(gameFilePath, nullptr);

    try {
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

        env->ReleaseStringUTFChars(gameFilePath, gamePath);

    } catch (std::exception& exception) {
        LibretroDroid::JavaUtils::throwRetroException(env, LibretroDroid::ERROR_LOAD_GAME);
    }
}

JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_loadGameFromBytes(
        JNIEnv * env,
        jobject obj,
        jbyteArray gameFileBytes
) {
    LOGD("Performing LibretroDroid loadGameFromBytes");

    try {
        struct retro_system_info system_info;
        core->retro_get_system_info(&system_info);

        struct retro_game_info game_info;
        game_info.path = nullptr;
        game_info.meta = nullptr;

        size_t gameFileSizeNative = env->GetArrayLength(gameFileBytes);
        auto* gameFileBytesNative = new char[gameFileSizeNative];
        env->GetByteArrayRegion(gameFileBytes, 0, gameFileSizeNative, reinterpret_cast<jbyte*>(gameFileBytesNative));

        if (system_info.need_fullpath) {
            game_info.data = nullptr;
            game_info.size = 0;
        } else {
            game_info.data = gameFileBytesNative;
            game_info.size = gameFileSizeNative;
        }

        bool result = core->retro_load_game(&game_info);
        if (!result) {
            LOGE("Cannot load game. Leaving.");
            throw std::runtime_error("Cannot load game");
        }
    } catch (std::exception& exception) {
        LibretroDroid::JavaUtils::throwRetroException(env, LibretroDroid::ERROR_LOAD_GAME);
    }
}

JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_destroy(JNIEnv * env, jobject obj) {
    LOGD("Performing LibretroDroid destroy");

    try {
        if (Environment::hw_context_destroy != nullptr) {
            Environment::hw_context_destroy();
        }

        core->retro_unload_game();
        core->retro_deinit();

        delete video;
        video = nullptr;

        delete core;
        core = nullptr;

        delete rumble;
        rumble = nullptr;

        Environment::deinitialize();

    } catch (std::exception& exception) {
        LibretroDroid::JavaUtils::throwRetroException(env, LibretroDroid::ERROR_GENERIC);
    }
}

JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_resume(JNIEnv * env, jobject obj) {
    LOGD("Performing LibretroDroid resume");

    try {
        input = new LibretroDroid::Input();

        struct retro_system_av_info system_av_info;
        core->retro_get_system_av_info(&system_av_info);

        fpsSync = new LibretroDroid::FPSSync(system_av_info.timing.fps, screenRefreshRate);

        double audioSamplingRate = system_av_info.timing.sample_rate / fpsSync->getTimeStretchFactor();
        audio = new LibretroDroid::Audio(std::lround(audioSamplingRate));
        updateAudioSampleRateMultiplier();
        audio->start();

    } catch (std::exception& exception) {
        LibretroDroid::JavaUtils::throwRetroException(env, LibretroDroid::ERROR_GENERIC);
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
        LibretroDroid::JavaUtils::throwRetroException(env, LibretroDroid::ERROR_GENERIC);
    }
}

void handlePostStepTasks(JNIEnv * env, jobject obj, jobject glRetroView) {
    if (rumble != nullptr) {
        rumble->updateAndDispatch(Environment::lastRumbleStrength, env, glRetroView);
    }

    // Some games override the core geometry at runtime. These fields get updated in retro_run().
    if (Environment::gameGeometryUpdated) {
        Environment::gameGeometryUpdated = false;
        jclass cls = env->GetObjectClass(glRetroView);
        jmethodID requestAspectRatioUpdate = env->GetMethodID(cls, "refreshAspectRatio", "()V");
        env->CallVoidMethod(glRetroView, requestAspectRatioUpdate);
    }
}

JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_step(JNIEnv * env, jobject obj, jobject glRetroView)
{
    LOGD("Stepping into retro_run()");

    retroStateMutex.lock();
    core->retro_run();
    if (fastForwardEnabled) {
        core->retro_run();
    }
    retroStateMutex.unlock();

    if (video != nullptr) {
        video->renderFrame();
    }

    if (fpsSync != nullptr) {
        fpsSync->sync();
    }

    handlePostStepTasks(env, obj, glRetroView);
}

float retrieveGameSpecificAspectRatio() {
    if (Environment::gameGeometryAspectRatio > 0) {
        return Environment::gameGeometryAspectRatio;
    }

    if (Environment::gameGeometryWidth > 0 && Environment::gameGeometryHeight > 0) {
        return (float) Environment::gameGeometryWidth / (float) Environment::gameGeometryHeight;
    }

    return -1.0f;
}

JNIEXPORT jfloat JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_getAspectRatio(JNIEnv * env, jobject obj) {
    float aspectRatio = retrieveGameSpecificAspectRatio();
    if (aspectRatio > 0) {
        return aspectRatio;
    }

    struct retro_system_av_info system_av_info;
    core->retro_get_system_av_info(&system_av_info);

    aspectRatio = system_av_info.geometry.aspect_ratio;
    if (aspectRatio > 0) {
        return aspectRatio;
    }

    aspectRatio = (float) system_av_info.geometry.base_width / (float) system_av_info.geometry.base_height;

    return aspectRatio;
}

JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_setRumbleEnabled(JNIEnv * env, jobject obj, jboolean enabled) {
    rumble->setEnabled(enabled);
}

JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_setFastForwardEnabled(JNIEnv * env, jobject obj, jboolean enabled) {
    fastForwardEnabled = enabled;
    updateAudioSampleRateMultiplier();
}

JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_setAudioEnabled(JNIEnv * env, jobject obj, jboolean enabled) {
    audioEnabled = enabled;
}
