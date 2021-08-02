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

#ifndef LIBRETRODROID_ENVIRONMENT_H
#define LIBRETRODROID_ENVIRONMENT_H

#define MODULE_NAME_CORE "Libretro Core"

#include <vector>
#include <string>
#include <cstring>
#include <cmath>
#include <EGL/egl.h>
#include <unordered_map>

#include "libretro/libretro.h"
#include "log.h"

namespace Environment {
    extern retro_hw_context_reset_t hw_context_reset;
    extern retro_hw_context_reset_t hw_context_destroy;
    extern struct retro_disk_control_callback* retro_disk_control_callback;

    extern const char* savesDirectory;
    extern const char* systemDirectory;
    extern retro_hw_get_current_framebuffer_t callback_get_current_framebuffer;
    extern unsigned language;

    extern int pixelFormat;
    extern bool useHWAcceleration;
    extern bool useDepth;
    extern bool useStencil;
    extern bool bottomLeftOrigin;
    extern float screenRotation;

    extern bool gameGeometryUpdated;
    extern unsigned gameGeometryWidth;
    extern unsigned gameGeometryHeight;
    extern float gameGeometryAspectRatio;

    extern uint16_t vibrationStrengthWeak;
    extern uint16_t vibrationStrengthStrong;
    extern uint16_t lastRumbleStrength;

    extern std::vector<struct Variable> variables;
    extern bool dirtyVariables;

    extern std::vector<std::vector<struct Controller>> controllers;

    struct Variable {
    public:
        std::string key;
        std::string value;
        std::string description;
    };

    struct Controller {
    public:
        unsigned id;
        std::string description;
    };

    void initialize(
        const char* requiredSystemDirectory,
        const char* requiredSavesDirectory,
        retro_hw_get_current_framebuffer_t required_callback_get_current_framebuffer
    );

    void deinitialize();

    void updateVariable(const std::string& key, const std::string& value);

    bool environment_handle_set_variables(const struct retro_variable* received);

    bool environment_handle_get_variable(struct retro_variable* requested);

    bool environment_handle_set_controller_info(const struct retro_controller_info* received);

    bool environment_handle_set_hw_render(struct retro_hw_render_callback* hw_render_callback);

    void callback_retro_log(enum retro_log_level level, const char *fmt, ...);

    bool set_rumble_state(unsigned port, enum retro_rumble_effect effect, uint16_t strength);

    bool callback_environment(unsigned cmd, void *data);

    void setLanguage(const std::string& androidLanguage);
}

#endif //LIBRETRODROID_ENVIRONMENT_H

