cmake_minimum_required(VERSION 3.26.3)

project(cef-tex)

include(FetchContent)
FetchContent_Declare(
    cef 
    GIT_REPOSITORY https://bitbucket.org/chromiumembedded/cef-project.git
    cef_BINARY_DIR cef-project
)

FetchContent_Declare(
    godot-cpp
    GIT_REPOSITORY https://github.com/godotengine/godot-cpp
    GIT_TAG 3.x
)

FetchContent_MakeAvailable(
    cef
)



FetchContent_MakeAvailable(godot-cpp)