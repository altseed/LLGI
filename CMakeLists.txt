cmake_minimum_required(VERSION 3.12)

# The project's name
project(LLGI)

set (CMAKE_CXX_STANDARD 11)

option(BUILD_METAL "build metal" OFF)
option(BUILD_VULKAN "build vulkan" OFF)

if(BUILD_VULKAN)
  add_definitions(-DENABLE_VULKAN)
endif()

if(BUILD_METAL)
  add_definitions(-DENABLE_METAL)
endif()

add_subdirectory("src")
add_subdirectory("src_test")

