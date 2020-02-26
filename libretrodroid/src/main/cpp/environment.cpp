/*
 *     Copyright (C) 2020  Filippo Scognamiglio
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

#include <vector>
#include <string>
#include <cstring>
#include <cmath>
#include <EGL/egl.h>

#include "libretro/libretro.h"
#include "log.h"

namespace Environment {
    retro_hw_context_reset_t hw_context_reset = nullptr;
    retro_hw_context_reset_t hw_context_destroy = nullptr;

    const char* savesDirectory = nullptr;
    const char* systemDirectory = nullptr;
    retro_hw_get_current_framebuffer_t callback_get_current_framebuffer = nullptr;

    int pixelFormat = RETRO_PIXEL_FORMAT_RGB565;
    bool useHWAcceleration = false;
    bool useDepth = false;
    bool useStencil = false;
    bool bottomLeftOrigin = false;
    float screenRotation = 0;

    std::vector<struct Variable> variables;
    bool dirtyVariables = false;

    struct Variable {
    public:
        std::string key;
        std::string value;
        std::string description;
    };

    void initialize(const char* systemDirectory, const char* savesDirectory, retro_hw_get_current_framebuffer_t callback_get_current_framebuffer) {
        Environment::callback_get_current_framebuffer = callback_get_current_framebuffer;
        Environment::systemDirectory = systemDirectory;
        Environment::savesDirectory = savesDirectory;
    }

    void deinitialize() {
        Environment::callback_get_current_framebuffer = nullptr;
        Environment::systemDirectory = nullptr;
        Environment::savesDirectory = nullptr;
        hw_context_reset = nullptr;
        hw_context_destroy = nullptr;
    }

    void updateVariable(std::string key, std::string value) {
        for (auto& variable : variables) {
            if (variable.key == key) {
                variable.value = value;
                dirtyVariables = true;
                break;
            }
        }
    }

    bool environment_handle_set_variables(const struct retro_variable* received) {
        variables.clear();

        unsigned count = 0;
        while (received[count].key != nullptr) {
            LOGD("Received variable %s: %s", received[count].key, received[count].value);

            std::string currentKey(received[count].key);
            std::string currentDescription(received[count].value);
            std::string currentValue(received[count].value);

            auto firstValueStart = currentValue.find(';') + 2;
            auto firstValueEnd = currentValue.find('|', firstValueStart);
            currentValue = currentValue.substr(firstValueStart, firstValueEnd - firstValueStart);

            auto variable = Variable { currentKey, currentValue, currentDescription };
            variables.push_back(variable);

            LOGD("Assigning variable %s: %s", variable.key.c_str(), variable.value.c_str());

            count++;
        }

        return true;
    }

    bool environment_handle_get_variable(struct retro_variable* requested) {
        LOGD("Variable requested %s", requested->key);

        // TODO... We should find a proper place for hardcoded properties like this one.
        // Desmume by defaults assumes a mouse pointer. We are forcing touchscreen which works best on Android.
        if (strcmp(requested->key, "desmume_pointer_type") == 0) {
            requested->value = "touch";
            return true;
        }

        // PCSXRearmed now uses Lightrec (which is awesome) but sadly this still doesn't work with HLE bioses,
        // so we are disabling it for now.
        if (strcmp(requested->key, "pcsx_rearmed_drc") == 0) {
            requested->value = "disabled";
            return true;
        }

        for (auto& variable : variables) {
            if (variable.key == requested->key) {
                requested->value = variable.value.c_str();
                return true;
            }
        }
        return false;
    }

    bool environment_handle_set_hw_render(struct retro_hw_render_callback* hw_render_callback) {
        useHWAcceleration = true;
        useDepth = hw_render_callback->depth;
        useStencil = hw_render_callback->stencil;
        bottomLeftOrigin = hw_render_callback->bottom_left_origin;

        hw_context_destroy = hw_render_callback->context_destroy;
        hw_context_reset = hw_render_callback->context_reset;
        hw_render_callback->get_current_framebuffer = callback_get_current_framebuffer;
        hw_render_callback->get_proc_address = &eglGetProcAddress;

        return true;
    }

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

    bool callback_environment(unsigned cmd, void *data) {
        switch (cmd) {
            case RETRO_ENVIRONMENT_GET_CAN_DUPE:
                *((bool*) data) = true;
                return true;

            case RETRO_ENVIRONMENT_SET_PIXEL_FORMAT: {
                LOGD("Called SET_PIXEL_FORMAT");
                pixelFormat = *static_cast<enum retro_pixel_format *>(data);
                return pixelFormat == RETRO_PIXEL_FORMAT_RGB565 || pixelFormat == RETRO_PIXEL_FORMAT_XRGB8888;
            }

            case RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS:
                LOGD("Called SET_INPUT_DESCRIPTORS");
                return false;

            case RETRO_ENVIRONMENT_GET_VARIABLE:
                LOGD("Called RETRO_ENVIRONMENT_GET_VARIABLE");
                return environment_handle_get_variable(static_cast<struct retro_variable*>(data));

            case RETRO_ENVIRONMENT_SET_VARIABLES:
                LOGD("Called RETRO_ENVIRONMENT_SET_VARIABLES");
                return environment_handle_set_variables(static_cast<const struct retro_variable*>(data));

            case RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE: {
                LOGD("Called RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE. Is dirty?: %d", dirtyVariables);
                *((bool*) data) = dirtyVariables;
                dirtyVariables = false;
                return true;
            }

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
                LOGD("Called RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY");
                *(const char**) data = systemDirectory;
                return systemDirectory != nullptr;

            case RETRO_ENVIRONMENT_SET_ROTATION: {
                LOGD("Called RETRO_ENVIRONMENT_SET_ROTATION");
                unsigned screenRotationIndex = (*static_cast<unsigned*>(data));
                screenRotation = screenRotationIndex * (float) (-M_PI / 2.0);
                return true;
            }

            case RETRO_ENVIRONMENT_GET_PERF_INTERFACE:
                LOGD("Called RETRO_ENVIRONMENT_GET_PERF_INTERFACE");
                return false;

            case RETRO_ENVIRONMENT_SET_GEOMETRY:
                LOGD("Called RETRO_ENVIRONMENT_SET_GEOMETRY");
                return false;

            case RETRO_ENVIRONMENT_SET_SYSTEM_AV_INFO:
                LOGD("Called RETRO_ENVIRONMENT_SET_SYSTEM_AV_INFO");
                return false;

            case RETRO_ENVIRONMENT_GET_LANGUAGE:
                LOGD("Called RETRO_ENVIRONMENT_GET_LANGUAGE");
                return false;
        }

        LOGI("callback environment has been called: %u", cmd);
        return false;
    }
}