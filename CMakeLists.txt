# Require a modern version of CMake
cmake_minimum_required(VERSION 3.25)

# Name our project
project(SpatialEngine3D)

# Allow dependencies with older policy baselines (e.g., glm 0.9.9.8) on modern CMake.
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

# Tell the compiler to use modern C++17 features
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include the tool that downloads libraries from the internet
include(FetchContent)

# 1. Fetch GLFW (This creates our Window and handles mouse/keyboard input)
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.3.8
)
FetchContent_MakeAvailable(glfw)

# 2. Fetch GLM (This is our "Numpy" for C++ 3D Math)
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 0.9.9.8
)
FetchContent_MakeAvailable(glm)

# 3. Find OpenGL (This is already installed on your computer's OS)
find_package(OpenGL REQUIRED)

# Tell CMake to compile our executable using main.cpp
add_executable(PathfinderEngine src/main.cpp)

# Tell CMake where our header (.h) files live
target_include_directories(PathfinderEngine PRIVATE include)

# Link everything together: Our App + GLFW + OpenGL + GLM
target_link_libraries(PathfinderEngine PRIVATE glfw OpenGL::GL glm)