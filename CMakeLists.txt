cmake_minimum_required(VERSION 3.12)

# The project's name
project(LLGI)

include(ExternalProject)
set (CMAKE_CXX_STANDARD 11)

# linux flag
if(UNIX AND NOT APPLE)
  set(LINUX TRUE)
endif()

option(BUILD_VULKAN "build vulkan" OFF)
option(BUILD_VULKAN_COMPILER "build vulkan compiler" OFF)
option(BUILD_TEST "build test" OFF)
option(BUILD_EXAMPLE "build examples" OFF)
option(BUILD_TOOL "build tools" OFF)
option(USE_THIRD_PARTY_DIRECTORY "Use a third party directory" ON)

if(LINUX)
  set(BUILD_VULKAN TRUE)
endif()

option(USE_MSVC_RUNTIME_LIBRARY_DLL "compile as multithreaded DLL" ON)

include(cmake/ClangFormat.cmake)

# MSVC Flag
if (MSVC)
  if (NOT USE_MSVC_RUNTIME_LIBRARY_DLL)
    foreach (flag CMAKE_C_FLAGS
      CMAKE_C_FLAGS_DEBUG
      CMAKE_C_FLAGS_RELEASE
      CMAKE_CXX_FLAGS
      CMAKE_CXX_FLAGS_DEBUG
      CMAKE_CXX_FLAGS_RELEASE)

      if (${flag} MATCHES "/MD")
        string(REGEX REPLACE "/MD" "/MT" ${flag} "${${flag}}")
      endif()
      if (${flag} MATCHES "/MDd")
        string(REGEX REPLACE "/MDd" "/MTd" ${flag} "${${flag}}")
      endif()
    endforeach()
  endif()
endif()

if(BUILD_VULKAN)
  add_definitions(-DENABLE_VULKAN)
  if(BUILD_VULKAN_COMPILER)
    add_definitions(-DENABLE_VULKAN_COMPILER)
    
    ExternalProject_Add(
        ExternalProject_glslang
        GIT_REPOSITORY  https://github.com/KhronosGroup/glslang.git
        GIT_TAG         7.12.3352
        SOURCE_DIR      ${CMAKE_CURRENT_BINARY_DIR}/ExternalProjects/glslang-src
        BINARY_DIR      ${CMAKE_CURRENT_BINARY_DIR}/ExternalProjects/glslang-build
        PREFIX          ${CMAKE_CURRENT_BINARY_DIR}/ExternalProjects/Build/glslang
        INSTALL_DIR     ${CMAKE_CURRENT_BINARY_DIR}/ExternalProjects/Install/glslang
        CMAKE_ARGS      -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR}/ExternalProjects/Install/glslang
    )
    list(APPEND THIRDPARTY_INCLUDES ${CMAKE_CURRENT_BINARY_DIR}/ExternalProjects/Install/glslang/include)
    list(APPEND THIRDPARTY_LIBRARY_DIRECTORIES ${CMAKE_CURRENT_BINARY_DIR}/ExternalProjects/Install/glslang/lib)
  endif()
endif()

if(APPLE)
  add_definitions(-DENABLE_METAL)
endif()

add_subdirectory("src")

if(BUILD_TEST)
  add_subdirectory("src_test")
endif()

if(BUILD_EXAMPLE)
  add_subdirectory("examples")
endif()

if(BUILD_TOOL)
  add_subdirectory("tools")
endif()