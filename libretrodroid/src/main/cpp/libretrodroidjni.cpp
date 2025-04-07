/*
 *     Copyright (C) 2021  Filippo Scognamiglio
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

#include <memory>
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
#include "utils/javautils.h"
#include "errorcodes.h"
#include "environment.h"
#include "renderers/es3/framebufferrenderer.h"
#include "renderers/es2/imagerendereres2.h"
#include "renderers/es3/imagerendereres3.h"
#include "utils/jnistring.h"

namespace libretrodroid {

extern "C" {
#include "utils/utils.h"
#include "../../libretro-common/include/libretro.h"
#include "utils/libretrodroidexception.h"
}

extern "C" {

JNIEXPORT jint JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_availableDisks(
    JNIEnv* env,
    jclass obj
) {
    try {
        return LibretroDroid::getInstance().availableDisks();
    } catch (std::exception &exception) {
        LOGE("Error in serializeState: %s", exception.what());
        JavaUtils::throwRetroException(env, ERROR_GENERIC);
    }
    return 0;
}

JNIEXPORT jint JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_currentDisk(
    JNIEnv* env,
    jclass obj
) {
    try {
        return LibretroDroid::getInstance().currentDisk();
    } catch (std::exception &exception) {
        LOGE("Error in serializeState: %s", exception.what());
        JavaUtils::throwRetroException(env, ERROR_GENERIC);
    }
    return 0;
}

JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_changeDisk(
    JNIEnv* env,
    jclass obj,
    jint index
) {
    try {
        return LibretroDroid::getInstance().changeDisk(index);
    } catch (std::exception &exception) {
        LOGE("Error in serializeState: %s", exception.what());
        JavaUtils::throwRetroException(env, ERROR_GENERIC);
    }
}

JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_updateVariable(
    JNIEnv* env,
    jclass obj,
    jobject variable
) {
    try {
        Variable v = JavaUtils::variableFromJava(env, variable);
        Environment::getInstance().updateVariable(v.key, v.value);
    } catch (std::exception &exception) {
        LOGE("Error in serializeState: %s", exception.what());
        JavaUtils::throwRetroException(env, ERROR_GENERIC);
    }
}

JNIEXPORT jobjectArray JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_getVariables(
    JNIEnv* env,
    jclass obj
) {
    try {
        jclass variableClass = env->FindClass("com/swordfish/libretrodroid/Variable");
        jmethodID variableMethodID = env->GetMethodID(variableClass, "<init>", "()V");

        auto variables = Environment::getInstance().getVariables();
        jobjectArray result = env->NewObjectArray(variables.size(), variableClass, nullptr);

        for (int i = 0; i < variables.size(); i++) {
            jobject jVariable = env->NewObject(variableClass, variableMethodID);

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
    } catch (std::exception &exception) {
        LOGE("Error in serializeState: %s", exception.what());
        JavaUtils::throwRetroException(env, ERROR_GENERIC);
    }
    return nullptr;
}

JNIEXPORT jobjectArray JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_getControllers(
    JNIEnv* env,
    jclass obj
) {
    try {
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
            jmethodID variableMethodID = env->GetMethodID(variableClass2, "<init>", "()V");

            for (int j = 0; j < controllers[i].size(); j++) {
                jobject jController = env->NewObject(variableClass2, variableMethodID);

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
    } catch (std::exception &exception) {
        LOGE("Error in serializeState: %s", exception.what());
        JavaUtils::throwRetroException(env, ERROR_GENERIC);
    }
    return nullptr;
}

JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_setControllerType(
    JNIEnv* env,
    jclass obj,
    jint port,
    jint type
) {
    try {
        LibretroDroid::getInstance().setControllerType(port, type);
    } catch (std::exception &exception) {
        LOGE("Error in serializeState: %s", exception.what());
        JavaUtils::throwRetroException(env, ERROR_GENERIC);
    }
}

JNIEXPORT jboolean JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_unserializeState(
    JNIEnv* env,
    jclass obj,
    jbyteArray state
) {
    try {
        jboolean isCopy = JNI_FALSE;
        jbyte* data = env->GetByteArrayElements(state, &isCopy);
        jsize size = env->GetArrayLength(state);

        bool result = LibretroDroid::getInstance().unserializeState(data, size);
        env->ReleaseByteArrayElements(state, data, JNI_ABORT);

        return result ? JNI_TRUE : JNI_FALSE;

    } catch (std::exception &exception) {
        LOGE("Error in unserializeState: %s", exception.what());
        JavaUtils::throwRetroException(env, ERROR_SERIALIZATION);
        return JNI_FALSE;
    }
}

JNIEXPORT jbyteArray JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_serializeState(
    JNIEnv* env,
    jclass obj
) {
    try {
        auto [data, size] = LibretroDroid::getInstance().serializeState();

        jbyteArray result = env->NewByteArray(size);
        env->SetByteArrayRegion(result, 0, size, data);

        return result;

    } catch (std::exception &exception) {
        LOGE("Error in serializeState: %s", exception.what());
        JavaUtils::throwRetroException(env, ERROR_SERIALIZATION);
    }

    return nullptr;
}

JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_setCheat(
    JNIEnv* env,
    jclass obj,
    jint index,
    jboolean enabled,
    jstring code
) {
    try {
        auto codeString = JniString(env, code);
        LibretroDroid::getInstance().setCheat(index, enabled, codeString.stdString());
    } catch (std::exception &exception) {
        LOGE("Error in setCheat: %s", exception.what());
        JavaUtils::throwRetroException(env, ERROR_CHEAT);
    }
}

JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_resetCheat(
    JNIEnv* env,
    jclass obj
) {
    try {
        LibretroDroid::getInstance().resetCheat();
    } catch (std::exception &exception) {
        LOGE("Error in resetCheat: %s", exception.what());
        JavaUtils::throwRetroException(env, ERROR_CHEAT);
    }
}

JNIEXPORT jboolean JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_unserializeSRAM(
    JNIEnv* env,
    jclass obj,
    jbyteArray sram
) {
    try {
        jboolean isCopy = JNI_FALSE;
        jbyte* data = env->GetByteArrayElements(sram, &isCopy);
        jsize size = env->GetArrayLength(sram);

        LibretroDroid::getInstance().unserializeSRAM(data, size);

        env->ReleaseByteArrayElements(sram, data, JNI_ABORT);

    } catch (std::exception &exception) {
        LOGE("Error in unserializeSRAM: %s", exception.what());
        JavaUtils::throwRetroException(env, ERROR_SERIALIZATION);
        return JNI_FALSE;
    }

    return JNI_TRUE;
}

JNIEXPORT jbyteArray JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_serializeSRAM(
    JNIEnv* env,
    jclass obj
) {
    try {
        auto [data, size] = LibretroDroid::getInstance().serializeSRAM();

        jbyteArray result = env->NewByteArray(size);
        env->SetByteArrayRegion(result, 0, size, (jbyte *) data);

        return result;

    } catch (std::exception &exception) {
        LOGE("Error in serializeSRAM: %s", exception.what());
        JavaUtils::throwRetroException(env, ERROR_SERIALIZATION);
    }

    return nullptr;
}

JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_reset(
    JNIEnv* env,
    jclass obj
) {
    try {
        LibretroDroid::getInstance().reset();
    } catch (std::exception &exception) {
        LOGE("Error in clear: %s", exception.what());
        JavaUtils::throwRetroException(env, ERROR_GENERIC);
    }
}

JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_onSurfaceChanged(
    JNIEnv* env,
    jclass obj,
    jint width,
    jint height
) {
    try {
        LibretroDroid::getInstance().onSurfaceChanged(width, height);
    } catch (std::exception &exception) {
        LOGE("Error in resume: %s", exception.what());
        JavaUtils::throwRetroException(env, ERROR_GENERIC);
    }
}

JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_onSurfaceCreated(
    JNIEnv* env,
    jclass obj
) {
    try {
        LibretroDroid::getInstance().onSurfaceCreated();
    } catch (std::exception &exception) {
        LOGE("Error in resume: %s", exception.what());
        JavaUtils::throwRetroException(env, ERROR_GENERIC);
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
    try {
        LibretroDroid::getInstance().onMotionEvent(port, source, xAxis, yAxis);
    } catch (std::exception &exception) {
        LOGE("Error in serializeState: %s", exception.what());
        JavaUtils::throwRetroException(env, ERROR_GENERIC);
    }
}

JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_onTouchEvent(
    JNIEnv* env,
    jclass obj,
    jfloat xAxis,
    jfloat yAxis
) {
    try {
        LibretroDroid::getInstance().onTouchEvent(xAxis, yAxis);
    } catch (std::exception &exception) {
        LOGE("Error in serializeState: %s", exception.what());
        JavaUtils::throwRetroException(env, ERROR_GENERIC);
    }
}

JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_onKeyEvent(
    JNIEnv* env,
    jclass obj,
    jint port,
    jint action,
    jint keyCode
) {
    try {
        LibretroDroid::getInstance().onKeyEvent(port, action, keyCode);
    } catch (std::exception &exception) {
        LOGE("Error in serializeState: %s", exception.what());
        JavaUtils::throwRetroException(env, ERROR_GENERIC);
    }
}

JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_create(
    JNIEnv* env,
    jclass obj,
    jint GLESVersion,
    jstring soFilePath,
    jstring systemDir,
    jstring savesDir,
    jobjectArray jVariables,
    jobject shaderConfig,
    jfloat refreshRate,
    jboolean preferLowLatencyAudio,
    jboolean enableVirtualFileSystem,
    jboolean enableMicrophone,
    jboolean skipDuplicateFrames,
    jboolean enableAmbientMode,
    jstring language
) {
    try {
        auto corePath = JniString(env, soFilePath);
        auto deviceLanguage = JniString(env, language);
        auto systemDirectory = JniString(env, systemDir);
        auto savesDirectory = JniString(env, savesDir);

        std::vector<Variable> variables;
        int size = env->GetArrayLength(jVariables);
        for (int i = 0; i < size; i++) {
            auto jVariable = (jobject) env->GetObjectArrayElement(jVariables, i);
            auto variable = JavaUtils::variableFromJava(env, jVariable);
            variables.push_back(variable);
        }

        LibretroDroid::getInstance().create(
            GLESVersion,
            corePath.stdString(),
            systemDirectory.stdString(),
            savesDirectory.stdString(),
            variables,
            JavaUtils::shaderFromJava(env, shaderConfig),
            refreshRate,
            preferLowLatencyAudio,
            enableVirtualFileSystem,
            enableMicrophone,
            skipDuplicateFrames,
            enableAmbientMode,
            deviceLanguage.stdString()
        );

    } catch (libretrodroid::LibretroDroidError& exception) {
        LOGE("Error in create: %s", exception.what());
        JavaUtils::throwRetroException(env, exception.getErrorCode());
    } catch (std::exception &exception) {
        LOGE("Error in create: %s", exception.what());
        JavaUtils::throwRetroException(env, ERROR_LOAD_LIBRARY);
    }
}

JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_loadGameFromPath(
    JNIEnv* env,
    jclass obj,
    jstring gameFilePath
) {
    try {
        if (gameFilePath == nullptr) {
            LOGE("DQC", "Error: gameFilePath is null");
            JavaUtils::throwRetroException(env, ERROR_LOAD_GAME);
            return;
        }

        auto gamePath = JniString(env, gameFilePath);
        LibretroDroid::getInstance().loadGameFromPath(gamePath.stdString());
    } catch (std::exception &exception) {
        LOGE("Error in loadGameFromPath: %s", exception.what());
        JavaUtils::throwRetroException(env, ERROR_LOAD_GAME);
    }
}

JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_loadGameFromBytes(
    JNIEnv* env,
    jclass obj,
    jbyteArray gameFileBytes
) {
    try {
        if (gameFileBytes == nullptr) {
            JavaUtils::throwRetroException(env, ERROR_LOAD_GAME);
            return;
        }

        size_t size = env->GetArrayLength(gameFileBytes);

        std::vector<int8_t> data(size);

        env->GetByteArrayRegion(
            gameFileBytes,
            0,
            size,
            data.data()
        );
        LibretroDroid::getInstance().loadGameFromBytes(data, size);
    } catch (std::exception &exception) {
        LOGE("Error in loadGameFromBytes: %s", exception.what());
        JavaUtils::throwRetroException(env, ERROR_LOAD_GAME);
    }
}

JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_loadGameFromVirtualFiles(
        JNIEnv* env,
        jclass obj,
        jobject virtualFileList
) {

    try {
        jclass detachedVirtualFileClass = env->FindClass("com/swordfish/libretrodroid/DetachedVirtualFile");
        if (!detachedVirtualFileClass) {
            LOGE("DQC", "Failed to find class: com/swordfish/libretrodroid/DetachedVirtualFile");
            JavaUtils::throwRetroException(env, ERROR_LOAD_GAME);
            return;
        }
        jmethodID getVirtualFileMethodID = env->GetMethodID(
                detachedVirtualFileClass,
                "getVirtualPath",
                "()Ljava/lang/String;"
        );
        if (!getVirtualFileMethodID) {
            LOGE("Failed to find DetachedVirtualFile class!");
            env->DeleteLocalRef(detachedVirtualFileClass);
            JavaUtils::throwRetroException(env, ERROR_LOAD_GAME);
            return;
        }
        jmethodID getFileDescriptorMethodID = env->GetMethodID(
                env->FindClass("com/swordfish/libretrodroid/DetachedVirtualFile"),
                "getFileDescriptor",
                "()I"
        );

        if (!getFileDescriptorMethodID) {
            LOGE("Failed to find getFileDescriptorMethodID class!");
            env->DeleteLocalRef(detachedVirtualFileClass);
            JavaUtils::throwRetroException(env, ERROR_LOAD_GAME);
            return;
        }

        std::vector<VFSFile> virtualFiles;

        JavaUtils::forEachOnJavaIterable(env, virtualFileList, [&](jobject item) {
            if (!item) {
                LOGE("DQC", "Encountered a null DetachedVirtualFile object in the iterable");
                return; // Skip this item
            }

            jstring jVirtualPath = (jstring) env->CallObjectMethod(item, getVirtualFileMethodID);

            if (!jVirtualPath) {
                LOGE("Virtual file path is null");
                return;
            }
            JniString virtualFileName(env, jVirtualPath);

            int fileDescriptor = env->CallIntMethod(item, getFileDescriptorMethodID);
            if (fileDescriptor < 0) {
                LOGE("Invalid file descriptor for: %s", virtualFileName.stdString().c_str());
                return;
            }
            virtualFiles.emplace_back(VFSFile(virtualFileName.stdString(), fileDescriptor));
        });

        LibretroDroid::getInstance().loadGameFromVirtualFiles(std::move(virtualFiles));
    } catch (std::exception &exception) {
        LOGE("Error in loadGameFromDescriptors: %s", exception.what());
        JavaUtils::throwRetroException(env, ERROR_LOAD_GAME);
    }
}

JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_destroy(
    JNIEnv* env,
    jclass obj
) {
    try {
        LibretroDroid::getInstance().destroy();
    } catch (std::exception &exception) {
        LOGE("Error in destroy: %s", exception.what());
        JavaUtils::throwRetroException(env, ERROR_GENERIC);
    }
}

JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_resume(
    JNIEnv* env,
    jclass obj
) {
    try {
        LibretroDroid::getInstance().resume();
    } catch (std::exception &exception) {
        LOGE("Error in resume: %s", exception.what());
        JavaUtils::throwRetroException(env, ERROR_GENERIC);
    }
}

JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_pause(
    JNIEnv* env,
    jclass obj
) {
    try {
        LibretroDroid::getInstance().pause();
    } catch (std::exception &exception) {
        LOGE("Error in pause: %s", exception.what());
        JavaUtils::throwRetroException(env, ERROR_GENERIC);
    }
}

JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_step(
    JNIEnv* env,
    jclass obj,
    jobject glRetroView
) {
    try {
        LibretroDroid::getInstance().step();

        if (LibretroDroid::getInstance().requiresVideoRefresh()) {
            LibretroDroid::getInstance().clearRequiresVideoRefresh();
            jclass cls = env->GetObjectClass(glRetroView);
            jmethodID requestAspectRatioUpdate = env->GetMethodID(cls, "refreshAspectRatio", "()V");
            env->CallVoidMethod(glRetroView, requestAspectRatioUpdate);
        }

        if (LibretroDroid::getInstance().isRumbleEnabled()) {
            LibretroDroid::getInstance().handleRumbleUpdates([&](int port, float weak, float strong) {
                jclass cls = env->GetObjectClass(glRetroView);
                jmethodID sendRumbleStrengthMethodID = env->GetMethodID(cls, "sendRumbleEvent", "(IFF)V");
                env->CallVoidMethod(glRetroView, sendRumbleStrengthMethodID, port, weak, strong);
            });
        }
    } catch (std::exception &exception) {
        LOGE("Error in resume: %s", exception.what());
        JavaUtils::throwRetroException(env, ERROR_GENERIC);
    }
}

JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_setRumbleEnabled(
    JNIEnv* env,
    jclass obj,
    jboolean enabled
) {
    try {
        LibretroDroid::getInstance().setRumbleEnabled(enabled);
    } catch (std::exception &exception) {
        LOGE("Error in setRumbleEnabled: %s", exception.what());
        JavaUtils::throwRetroException(env, ERROR_GENERIC);
    }
}

JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_setFrameSpeed(
    JNIEnv* env,
    jclass obj,
    jint speed
) {
    try {
        LibretroDroid::getInstance().setFrameSpeed(speed);
    } catch (std::exception &exception) {
        LOGE("Error in setFrameSpeed: %s", exception.what());
        JavaUtils::throwRetroException(env, ERROR_GENERIC);
    }
}

JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_setAudioEnabled(
    JNIEnv* env,
    jclass obj,
    jboolean enabled
) {
    try {
        LibretroDroid::getInstance().setAudioEnabled(enabled);
    } catch (std::exception &exception) {
        LOGE("Error in setAudioEnabled: %s", exception.what());
        JavaUtils::throwRetroException(env, ERROR_GENERIC);
    }
}

JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_setShaderConfig(
    JNIEnv* env,
    jclass obj,
    jobject shaderConfig
) {
    try {
        LibretroDroid::getInstance().setShaderConfig(JavaUtils::shaderFromJava(env, shaderConfig));
    } catch (std::exception &exception) {
        LOGE("Error in setShaderConfig: %s", exception.what());
        JavaUtils::throwRetroException(env, ERROR_GENERIC);
    }
}

JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_setViewport(
    JNIEnv* env,
    jclass obj,
    jfloat x,
    jfloat y,
    jfloat width,
    jfloat height
) {
    try {
        LibretroDroid::getInstance().setViewport(Rect(x, y, width, height));
    } catch (std::exception &exception) {
        LOGE("Error in setViewport: %s", exception.what());
        JavaUtils::throwRetroException(env, ERROR_GENERIC);
    }
}

JNIEXPORT void JNICALL Java_com_swordfish_libretrodroid_LibretroDroid_refreshAspectRatio(
    JNIEnv* env,
    jclass obj
) {
    try {
        LibretroDroid::getInstance().refreshAspectRatio();
    } catch (std::exception &exception) {
        LOGE("Error in refreshAspectRatio: %s", exception.what());
        JavaUtils::throwRetroException(env, ERROR_GENERIC);
    }
}

}

}
