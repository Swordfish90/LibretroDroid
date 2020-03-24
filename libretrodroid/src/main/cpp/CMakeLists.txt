cmake_minimum_required(VERSION 3.4.1)

# now build app's shared lib
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall")

# Let's include oboe
set (OBOE_DIR oboe)
add_subdirectory (${OBOE_DIR} oboe)
include_directories (${OBOE_DIR}/include)

add_library(libretrodroid SHARED
        libretrodroid.cpp
        log.h
        core.h
        core.cpp
        video.h
        video.cpp
        renderers/renderer.h
        renderers/renderer.cpp
        renderers/es3/framebufferrenderer.h
        renderers/es3/framebufferrenderer.cpp
        renderers/es2/imagerendereres2.h
        renderers/es2/imagerendereres2.cpp
        renderers/es3/imagerendereres3.h
        renderers/es3/imagerendereres3.cpp
        audio.h
        audio.cpp
        fpssync.h
        fpssync.cpp
        input.h
        input.cpp
        shadermanager.h
        shadermanager.cpp
        javautils.h
        javautils.cpp
        utils.cpp
        utils.h
        libretro/libretro.h)

# add lib dependencies
target_link_libraries(libretrodroid
                      android
                      log 
                      EGL
                      oboe
                      GLESv3)

