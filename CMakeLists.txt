cmake_minimum_required(VERSION 3.16)
project(OneMoreMove CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Configure SDL3 FetchContent
include(FetchContent)
FetchContent_Declare(
    SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG release-3.2.0
)

# Speed up SDL3 build by disabling unnecessary components for static link
set(SDL_SHARED OFF CACHE BOOL "Build SDL shared library" FORCE)
set(SDL_STATIC ON CACHE BOOL "Build SDL static library" FORCE)
set(SDL_TESTS OFF CACHE BOOL "Build SDL test programs" FORCE)

FetchContent_MakeAvailable(SDL3)

# App source files
file(GLOB_RECURSE SOURCES 
    "src/*.cpp"
    "src/*.h"
)

# App target
if(ANDROID)
    add_library(OneMoreMove SHARED ${SOURCES})
else()
    add_executable(OneMoreMove ${SOURCES})
endif()

# Link against SDL3 library
target_link_libraries(OneMoreMove PRIVATE SDL3::SDL3-static)
target_include_directories(OneMoreMove PRIVATE src)

# Set startup project for Visual Studio
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT OneMoreMove)
