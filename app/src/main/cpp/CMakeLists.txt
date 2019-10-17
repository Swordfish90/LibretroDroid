cmake_minimum_required(VERSION 3.4.1)

# now build app's shared lib
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall")

# Let's include oboe
set (OBOE_DIR oboe)
add_subdirectory (${OBOE_DIR} ./oboe)
include_directories (${OBOE_DIR}/include)

add_library(gl2jni SHARED
        gl_code.cpp
        utils.c
        utils.h
        libretro.h)

# add lib dependencies
target_link_libraries(gl2jni
                      android
                      log 
                      EGL
                      oboe
                      GLESv3)

