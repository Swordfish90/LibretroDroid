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

#include "libretrodroid.h"
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
        audio->setSampleRateMultiplier(frameSpeed);
    }
}

void handlePostStepTasks(JNIEnv* env, jclass obj, jobject glRetroView) {
    if (rumble != nullptr) {
        rumble->updateAndDispatch(Environment::getInstance().getLastRumbleStrength(), env, glRetroView);
    }

    // Some games override the core geometry at runtime. These fields get updated in retro_run().
    if (Environment::getInstance().isGameGeometryUpdated()) {
        Environment::getInstance().clearGameGeometryUpdated();

        video->updateRendererSize(
            Environment::getInstance().getGameGeometryWidth(),
            Environment::getInstance().getGameGeometryHeight()
        );

        jclass cls = env->GetObjectClass(glRetroView);
        jmethodID requestAspectRatioUpdate = env->GetMethodID(cls, "refreshAspectRatio", "()V");
        env->CallVoidMethod(glRetroView, requestAspectRatioUpdate);
    }
}

float retrieveGameSpecificAspectRatio() {
    if (Environment::getInstance().getGameGeometryAspectRatio() > 0) {
        return Environment::getInstance().getGameGeometryAspectRatio();
    }

    if (Environment::getInstance().getGameGeometryWidth() > 0 && Environment::getInstance().getGameGeometryHeight() > 0) {
        return (float) Environment::getInstance().getGameGeometryWidth() / (float) Environment::getInstance().getGameGeometryHeight();
    }

    return -1.0f;
}

void resetGlobalVariables() {
    core = nullptr;
    audio = nullptr;
    video = nullptr;
    fpsSync = nullptr;
    input = nullptr;
    rumble = nullptr;
}

extern "C" {

JNIEXPORT jint JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_availableDisks(
    JNIEnv* env,
    jclass obj
) {
    std::lock_guard<std::mutex> lock(retroStateMutex);
    return Environment::getInstance().getRetroDiskControlCallback() != nullptr
           ? Environment::getInstance().getRetroDiskControlCallback()->get_num_images()
           : 0;
}

JNIEXPORT jint JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_currentDisk(
    JNIEnv* env,
    jclass obj
) {
    std::lock_guard<std::mutex> lock(retroStateMutex);
    return Environment::getInstance().getRetroDiskControlCallback() != nullptr
           ? Environment::getInstance().getRetroDiskControlCallback()->get_image_index()
           : 0;
}

JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_changeDisk(
    JNIEnv* env,
    jclass obj,
    jint index
) {
    std::lock_guard<std::mutex> lock(retroStateMutex);
    if (Environment::getInstance().getRetroDiskControlCallback() == nullptr) {
        LOGE("Cannot swap disk. This platform does not support it.");
        return;
    }

    if (index < 0 || index >= Environment::getInstance().getRetroDiskControlCallback()->get_num_images()) {
        LOGE("Requested image index is not valid.");
        return;
    }

    if (Environment::getInstance().getRetroDiskControlCallback()->get_image_index() != index) {
        Environment::getInstance().getRetroDiskControlCallback()->set_eject_state(true);
        Environment::getInstance().getRetroDiskControlCallback()->set_image_index((unsigned) index);
        Environment::getInstance().getRetroDiskControlCallback()->set_eject_state(false);
    }
}

JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_updateVariable(
    JNIEnv* env,
    jclass obj,
    jobject variable
) {
    jclass variableClass = env->FindClass("com/swordfish/libretrodroid/Variable");

    jfieldID jKeyField = env->GetFieldID(variableClass, "key", "Ljava/lang/String;");
    jfieldID jValueField = env->GetFieldID(variableClass, "value", "Ljava/lang/String;");

    auto jKeyObject = (jstring) env->GetObjectField(variable, jKeyField);
    auto jValueObject = (jstring) env->GetObjectField(variable, jValueField);

    jboolean isCopy = JNI_TRUE;

    Environment::getInstance().updateVariable(
        env->GetStringUTFChars(jKeyObject, &isCopy),
        env->GetStringUTFChars(jValueObject, &isCopy)
    );
}

JNIEXPORT jobjectArray JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_getVariables(
    JNIEnv* env,
    jclass obj
) {
    jclass variableClass = env->FindClass("com/swordfish/libretrodroid/Variable");
    jmethodID variableMethod = env->GetMethodID(variableClass, "<init>", "()V");

    auto variables = Environment::getInstance().getVariables();
    jobjectArray result = env->NewObjectArray(variables.size(), variableClass, nullptr);

    for (int i = 0; i < variables.size(); i++) {
        jobject jVariable = env->NewObject(variableClass, variableMethod);

        jfieldID jKeyField = env->GetFieldID(variableClass, "key", "Ljava/lang/String;");
        jfieldID jValueField = env->GetFieldID(variableClass, "value", "Ljava/lang/String;");
        jfieldID jDescriptionField = env->GetFieldID(
            variableClass,
            "description",
            "Ljava/lang/String;"
        );

        env->SetObjectField(jVariable, jKeyField, env->NewStringUTF(variables[i].key.data()));
        env->SetObjectField(jVariable, jValueField, env->NewStringUTF(variables[i].value.data()));
        env->SetObjectField(
            jVariable,
            jDescriptionField,
            env->NewStringUTF(variables[i].description.data()));

        env->SetObjectArrayElement(result, i, jVariable);
    }
    return result;
}

JNIEXPORT jobjectArray JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_getControllers(
    JNIEnv* env,
    jclass obj
) {
    jclass variableClass = env->FindClass("[Lcom/swordfish/libretrodroid/Controller;");

    auto controllers = Environment::getInstance().getControllers();
    jobjectArray result = env->NewObjectArray(controllers.size(), variableClass, nullptr);

    for (int i = 0; i < controllers.size(); i++) {
        jclass variableClass2 = env->FindClass("com/swordfish/libretrodroid/Controller");
        jobjectArray controllerArray = env->NewObjectArray(
            controllers[i].size(),
            variableClass2,
            nullptr
        );
        jmethodID variableMethod = env->GetMethodID(variableClass2, "<init>", "()V");

        for (int j = 0; j < controllers[i].size(); j++) {
            jobject jController = env->NewObject(variableClass2, variableMethod);

            jfieldID jIdField = env->GetFieldID(variableClass2, "id", "I");
            jfieldID jDescriptionField = env->GetFieldID(
                variableClass2,
                "description",
                "Ljava/lang/String;"
            );

            env->SetIntField(jController, jIdField, (int) controllers[i][j].id);
            env->SetObjectField(
                jController,
                jDescriptionField,
                env->NewStringUTF(controllers[i][j].description.data()));

            env->SetObjectArrayElement(controllerArray, j, jController);
        }

        env->SetObjectArrayElement(result, i, controllerArray);
    }
    return result;
}

JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_setControllerType(
    JNIEnv* env,
    jclass obj,
    jint port,
    jint type
) {
    core->retro_set_controller_port_device(port, type);
}

JNIEXPORT jboolean JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_unserializeState(
    JNIEnv* env,
    jclass obj,
    jbyteArray data
) {
    std::lock_guard<std::mutex> lock(retroStateMutex);

    try {
        jboolean isCopy = JNI_FALSE;
        jbyte *cData = env->GetByteArrayElements(data, &isCopy);
        jsize stateSize = env->GetArrayLength(data);

        bool result = core->retro_unserialize(cData, (size_t) stateSize);
        env->ReleaseByteArrayElements(data, cData, JNI_ABORT);

        return result ? JNI_TRUE : JNI_FALSE;

    } catch (std::exception &exception) {
        LibretroDroid::JavaUtils::throwRetroException(env, LibretroDroid::ERROR_SERIALIZATION);
        return JNI_FALSE;
    }
}

JNIEXPORT jbyteArray JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_serializeState(
    JNIEnv* env,
    jclass obj
) {
    std::lock_guard<std::mutex> lock(retroStateMutex);

    try {
        size_t size = core->retro_serialize_size();
        jbyte *state = new jbyte[size];

        core->retro_serialize(state, size);

        jbyteArray result = env->NewByteArray(size);
        env->SetByteArrayRegion(result, 0, size, state);

        return result;

    } catch (std::exception &exception) {
        LibretroDroid::JavaUtils::throwRetroException(env, LibretroDroid::ERROR_SERIALIZATION);
    }

    return nullptr;
}

JNIEXPORT jboolean JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_unserializeSRAM(
    JNIEnv* env,
    jclass obj,
    jbyteArray data
) {
    std::lock_guard<std::mutex> lock(retroStateMutex);

    try {
        jboolean isCopy = JNI_FALSE;
        jbyte *cData = env->GetByteArrayElements(data, &isCopy);
        jsize dataSize = env->GetArrayLength(data);

        size_t sramSize = core->retro_get_memory_size(RETRO_MEMORY_SAVE_RAM);
        void *sramState = core->retro_get_memory_data(RETRO_MEMORY_SAVE_RAM);

        if (sramState == nullptr) {
            LOGE("Cannot load SRAM: nullptr in retro_get_memory_data");
            env->ReleaseByteArrayElements(data, cData, JNI_ABORT);
            return false;
        }

        if (dataSize > sramSize) {
            LOGE("Cannot load SRAM: size mismatch");
            env->ReleaseByteArrayElements(data, cData, JNI_ABORT);
            return false;
        }

        memcpy(sramState, cData, dataSize);
        env->ReleaseByteArrayElements(data, cData, JNI_ABORT);

    } catch (std::exception &exception) {
        LibretroDroid::JavaUtils::throwRetroException(env, LibretroDroid::ERROR_SERIALIZATION);
        return false;
    }

    return true;
}

JNIEXPORT jbyteArray JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_serializeSRAM(
    JNIEnv* env,
    jclass obj
) {
    std::lock_guard<std::mutex> lock(retroStateMutex);

    try {
        size_t size = core->retro_get_memory_size(RETRO_MEMORY_SAVE_RAM);
        void *sram = core->retro_get_memory_data(RETRO_MEMORY_SAVE_RAM);

        jbyteArray result = env->NewByteArray(size);
        env->SetByteArrayRegion(result, 0, size, (jbyte *) sram);

        return result;

    } catch (std::exception &exception) {
        LibretroDroid::JavaUtils::throwRetroException(env, LibretroDroid::ERROR_SERIALIZATION);
    }

    return nullptr;
}

JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_reset(
    JNIEnv* env,
    jclass obj
) {
    std::lock_guard<std::mutex> lock(retroStateMutex);

    try {
        core->retro_reset();
    } catch (std::exception &exception) {
        LibretroDroid::JavaUtils::throwRetroException(env, LibretroDroid::ERROR_GENERIC);
    }
}

JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_onSurfaceChanged(
    JNIEnv* env,
    jclass obj,
    jint width,
    jint height
) {
    LOGD("Performing LibretroDroid onSurfaceChanged");
    video->updateScreenSize(width, height);
}

JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_onSurfaceCreated(
    JNIEnv* env,
    jclass obj
) {
    LOGD("Performing LibretroDroid onSurfaceCreated");

    struct retro_system_av_info system_av_info;
    core->retro_get_system_av_info(&system_av_info);

    if (video != nullptr) {
        delete video;
        video = nullptr;
    }

    LibretroDroid::Renderer *renderer;
    if (Environment::getInstance().isUseHwAcceleration()) {
        renderer = new LibretroDroid::FramebufferRenderer(
            system_av_info.geometry.base_width,
            system_av_info.geometry.base_height,
            Environment::getInstance().isUseDepth(),
            Environment::getInstance().isUseStencil()
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
        Environment::getInstance().isBottomLeftOrigin(),
        Environment::getInstance().getScreenRotation()
    );

    renderer->setPixelFormat(Environment::getInstance().getPixelFormat());

    video = newVideo;

    if (Environment::getInstance().getHwContextReset() != nullptr) {
        Environment::getInstance().getHwContextReset();
    }
}

JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_onMotionEvent(
    JNIEnv* env,
    jclass obj,
    jint port,
    jint source,
    jfloat xAxis,
    jfloat yAxis
) {
    LOGD("Received motion event: %d %.2f, %.2f", source, xAxis, yAxis);
    if (input != nullptr) {
        input->onMotionEvent(port, source, xAxis, yAxis);
    }
}

JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_onKeyEvent(
    JNIEnv* env,
    jclass obj,
    jint port,
    jint action,
    jint keyCode
) {
    LOGD("Received key event with action (%d) and keycode (%d)", action, keyCode);
    if (input != nullptr) {
        input->onKeyEvent(port, action, keyCode);
    }
}

JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_create(
    JNIEnv* env,
    jclass obj,
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
    const char *corePath = env->GetStringUTFChars(soFilePath, nullptr);
    const char *deviceLanguage = env->GetStringUTFChars(language, nullptr);

    resetGlobalVariables();

    try {
        Environment::getInstance().initialize(
            env->GetStringUTFChars(systemDir, nullptr),
            env->GetStringUTFChars(savesDir, nullptr),
            &callback_get_current_framebuffer
        );

        Environment::getInstance().setLanguage(std::string(deviceLanguage));

        openglESVersion = GLESVersion;
        screenRefreshRate = refreshRate;
        audioEnabled = true;
        frameSpeed = 1;

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
        if (Environment::getInstance().isUseHwAcceleration() && openglESVersion < 3) {
            LibretroDroid::JavaUtils::throwRetroException(
                env,
                LibretroDroid::ERROR_GL_NOT_COMPATIBLE
            );
        }

        env->ReleaseStringUTFChars(soFilePath, corePath);
        env->ReleaseStringUTFChars(language, deviceLanguage);

        fragmentShaderType = LibretroDroid::ShaderManager::Type(shaderType);

        rumble = new LibretroDroid::Rumble();

    } catch (std::exception &exception) {
        LibretroDroid::JavaUtils::throwRetroException(env, LibretroDroid::ERROR_LOAD_LIBRARY);
    }
}

JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_loadGameFromPath(
    JNIEnv* env,
    jclass obj,
    jstring gameFilePath
) {
    LOGD("Performing LibretroDroid loadGameFromPath");
    const char *gamePath = env->GetStringUTFChars(gameFilePath, nullptr);

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
            struct LibretroDroid::Utils::ReadResult file = LibretroDroid::Utils::readFileAsBytes(
                gamePath
            );
            game_info.data = file.data;
            game_info.size = file.size;
        }

        bool result = core->retro_load_game(&game_info);
        if (!result) {
            LOGE("Cannot load game. Leaving.");
            throw std::runtime_error("Cannot load game");
        }

        env->ReleaseStringUTFChars(gameFilePath, gamePath);

    } catch (std::exception &exception) {
        LibretroDroid::JavaUtils::throwRetroException(env, LibretroDroid::ERROR_LOAD_GAME);
    }
}

JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_loadGameFromBytes(
    JNIEnv* env,
    jclass obj,
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
        auto *gameFileBytesNative = new char[gameFileSizeNative];
        env->GetByteArrayRegion(
            gameFileBytes,
            0,
            gameFileSizeNative,
            reinterpret_cast<jbyte *>(gameFileBytesNative));

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
    } catch (std::exception &exception) {
        LibretroDroid::JavaUtils::throwRetroException(env, LibretroDroid::ERROR_LOAD_GAME);
    }
}

JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_destroy(
    JNIEnv* env,
    jclass obj
) {
    LOGD("Performing LibretroDroid destroy");

    try {
        if (Environment::getInstance().getHwContextDestroy() != nullptr) {
            Environment::getInstance().getHwContextDestroy();
        }

        core->retro_unload_game();
        core->retro_deinit();

        if (video) {
            delete video;
            video = nullptr;
        }

        if (core) {
            delete core;
            core = nullptr;
        }

        if (rumble) {
            delete rumble;
            rumble = nullptr;
        }

        Environment::getInstance().deinitialize();

    } catch (std::exception &exception) {
        LibretroDroid::JavaUtils::throwRetroException(env, LibretroDroid::ERROR_GENERIC);
    }
}

JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_resume(
    JNIEnv* env,
    jclass obj
) {
    LOGD("Performing LibretroDroid resume");

    try {
        input = new LibretroDroid::Input();

        struct retro_system_av_info system_av_info;
        core->retro_get_system_av_info(&system_av_info);

        fpsSync = new LibretroDroid::FPSSync(system_av_info.timing.fps, screenRefreshRate);

        double audioSamplingRate =
            system_av_info.timing.sample_rate / fpsSync->getTimeStretchFactor();
        audio = new LibretroDroid::Audio(std::lround(audioSamplingRate));
        updateAudioSampleRateMultiplier();
        audio->start();

    } catch (std::exception &exception) {
        LibretroDroid::JavaUtils::throwRetroException(env, LibretroDroid::ERROR_GENERIC);
    }
}

JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_pause(
    JNIEnv* env,
    jclass obj
) {
    LOGD("Performing LibretroDroid pause");

    try {
        if (input) {
            delete input;
            input = nullptr;
        }

        if (audio) {
            audio->stop();
            delete audio;
            audio = nullptr;
        }

        if (fpsSync) {
            delete fpsSync;
            fpsSync = nullptr;
        }

    } catch (std::exception &exception) {
        LibretroDroid::JavaUtils::throwRetroException(env, LibretroDroid::ERROR_GENERIC);
    }
}

JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_step(
    JNIEnv* env,
    jclass obj,
    jobject glRetroView
) {
    LOGD("Stepping into retro_run()");

    retroStateMutex.lock();
    unsigned frames = 1;
    if (fpsSync != nullptr) {
        unsigned requestedFrames = fpsSync->advanceFrames();

        // If the application runs too slow it's better to just skip those frames.
        frames = std::min(requestedFrames, 2u);
    }

    for (size_t i = 0; i < frames * frameSpeed; i++)
        core->retro_run();

    retroStateMutex.unlock();

    if (video != nullptr) {
        video->renderFrame();
    }

    if (fpsSync != nullptr) {
        fpsSync->wait();
    }

    handlePostStepTasks(env, obj, glRetroView);
}

JNIEXPORT jfloat JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_getAspectRatio(
    JNIEnv* env,
    jclass obj
) {
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

    aspectRatio =
        (float) system_av_info.geometry.base_width / (float) system_av_info.geometry.base_height;

    return aspectRatio;
}

JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_setRumbleEnabled(
    JNIEnv* env,
    jclass obj,
    jboolean enabled
) {
    rumble->setEnabled(enabled);
}

JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_setFrameSpeed(
    JNIEnv* env,
    jclass obj,
    jint speed
) {
    frameSpeed = speed;
    updateAudioSampleRateMultiplier();
}

JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_setAudioEnabled(
    JNIEnv* env,
    jclass obj,
    jboolean enabled
) {
    audioEnabled = enabled;
}

}
