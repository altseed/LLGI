# The project's name
project(LLGI)

set (CMAKE_CXX_STANDARD 11)

option(BUILD_METAL "build metal" OFF)
option(BUILD_VULKAN "build vulkan" OFF)

add_subdirectory("src")
add_subdirectory("src_test")

