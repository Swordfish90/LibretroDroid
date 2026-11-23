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
#include <memory>
#include <optional>

#include "log.h"
#include "core.h"
#include "audio.h"
#include "video.h"
#include "sharedcontext.h"
#include "renderers/renderer.h"
#include "fpssync.h"
#include "input.h"
#include "rumble.h"
#include "shadermanager.h"
#include "utils/javautils.h"
#include "environment.h"
#include "vfs/vfsfile.h"
#include "renderers/es3/framebufferrenderer.h"
#include "renderers/es2/imagerendereres2.h"
#include "renderers/es3/imagerendereres3.h"
#include "utils/rect.h"

namespace libretrodroid {

class LibretroDroid {
public:
    static LibretroDroid& getInstance()
    {
        static LibretroDroid instance;
        return instance;
    }
    LibretroDroid(LibretroDroid const&) = delete;
    void operator=(LibretroDroid const&) = delete;

private:
    LibretroDroid() {}

public:
    void create(
        unsigned int GLESVersion,
        const std::string& soFilePath,
        const std::string& systemDir,
        const std::string& savesDir,
        std::vector<Variable> variables,
        const ShaderManager::Config& shaderConfig,
        float refreshRate,
        bool lowLatencyAudio,
        bool enableVirtualFileSystem,
        bool enableMicrophone,
        std::optional<ImmersiveMode::Config> immersiveModeConfig,
        const std::string& language
    );
    void resume();
    bool step();
    void pause();
    void destroy();

    void reset();

    void loadGameFromPath(const std::string &gamePath);
    void loadGameFromBytes(const int8_t *data, size_t size);
    void loadGameFromVirtualFiles(std::vector<VFSFile> virtualFiles);

    std::vector<int8_t> serializeState();
    bool unserializeState(int8_t *data, size_t size);

    std::vector<int8_t> serializeSRAM();
    bool unserializeSRAM(int8_t *data, size_t size);

    void setCheat(unsigned index, bool enabled, const std::string& code);
    void resetCheat();

    void onKeyEvent(unsigned int port, int action, int keyCode);
    void onMotionEvent(unsigned int port, unsigned int source, float xAxis, float yAxis);
    void onTouchEvent(float xAxis, float yAxis);

    std::vector<std::vector<struct Controller>> getControllers();
    void setControllerType(unsigned int port, unsigned int type);

    void onContextCreated();
    void onSurfaceChanged(unsigned int width, unsigned int height);
    void onContextDestroyed();

    void refreshAspectRatio();
    float getAspectRatio();

    void setViewport(Rect viewportRect);
    void setViewportAlignment(unsigned int viewportAlignment);

    void setShaderConfig(ShaderManager::Config shaderConfig);

    void setFrameSpeed(unsigned int speed);

    void setAudioEnabled(bool enabled);

    std::vector<Variable> getVariables();
    void updateVariable(const Variable& variable);
    int availableDisks();

    int currentDisk();
    void changeDisk(unsigned int index);

    void setRumbleEnabled(bool enabled);
    bool isRumbleEnabled() const;
    void handleRumbleUpdates(const std::function<void(int, float, float)> &handler);

    void handleVideoRefresh(const void *data, unsigned width, unsigned height, size_t pitch);
    size_t handleAudioCallback(const int16_t* data, size_t frames);
    int16_t handleSetInputState(unsigned port, unsigned device, unsigned index, unsigned id);
    uintptr_t handleGetCurrentFrameBuffer();

private:
    enum class InitializationStage { New, CoreInitialized, GameLoaded, GraphicsInitialized };

    void updateAudioSampleRateMultiplier();
    float findDefaultAspectRatio(const retro_system_av_info &system_av_info);
    void initializeCore(const std::string& soFilePath, const std::vector<Variable>& variables);
    void afterGameLoad();

protected:
    static void callback_hw_video_refresh(const void *data, unsigned width, unsigned height, size_t pitch);
    static size_t callback_set_audio_sample_batch(const int16_t* data, size_t frames);
    static void callback_audio_sample(int16_t left, int16_t right);
    static int16_t callback_set_input_state(unsigned port, unsigned device, unsigned index, unsigned id);
    static uintptr_t callback_get_current_framebuffer();
    static void callback_retro_set_input_poll();

private:
    unsigned int frameSpeed = 1;
    bool audioEnabled = true;
    bool preferLowLatencyAudio = false;
    bool rumbleEnabled = false;

    ShaderManager::Config fragmentShaderConfig = ShaderManager::Config {
        ShaderManager::Type::SHADER_DEFAULT, { }
    };

    Rect viewportRect = Rect(0.0F, 0.0F, 1.0F, 1.0F);
    unsigned int viewportAlignment = V_ALIGN_CENTER;
    float screenRefreshRate = 60.0;
    int openglESVersion = 2;
    bool immersiveModeEnabled = false;
    ImmersiveMode::Config immersiveModeConfig {};

    float defaultAspectRatio = 1.0;
    bool dirtyVideo = false;
    bool frameSubmitted = false;
    InitializationStage initializationStage = InitializationStage::New;

    std::unique_ptr<Core> core;
    std::unique_ptr<Audio> audio;
    std::unique_ptr<Video> video;
    std::unique_ptr<SharedContext> sharedContext;
    std::unique_ptr<FPSSync> fpsSync;
    std::unique_ptr<Input> input;
    std::unique_ptr<Rumble> rumble;
};

} //namespace libretrodroid

#endif //LIBRETRODROID_LIBRETRODROID_H
