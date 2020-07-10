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
option(USE_THIRDPARTY_DIRECTORY "Whether do it compile with third party directory" ON)

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

if((BUILD_VULKAN_COMPILER OR BUILD_TOOL) AND USE_THIRDPARTY_DIRECTORY)
  ExternalProject_Add(
      EP_glslang
      SOURCE_DIR      ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/glslang
      PREFIX          ${CMAKE_CURRENT_BINARY_DIR}/EP/Build/glslang
      INSTALL_DIR     ${CMAKE_CURRENT_BINARY_DIR}/EP/Install/glslang
      CMAKE_ARGS      -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR}/EP/Install/glslang
                      -DCMAKE_USER_MAKE_RULES_OVERRIDE=${CMAKE_CURRENT_SOURCE_DIR}/cmake/CFlagOverrides.cmake
                      -DUSE_MSVC_RUNTIME_LIBRARY_DLL=${USE_MSVC_RUNTIME_LIBRARY_DLL}
                      -DCMAKE_DEBUG_POSTFIX=d
                      -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
                      -DCMAKE_POSITION_INDEPENDENT_CODE=ON
  )
  list(APPEND LLGI_THIRDPARTY_INCLUDES ${CMAKE_CURRENT_BINARY_DIR}/EP/Install/glslang/include)
  list(APPEND LLGI_THIRDPARTY_LIBRARY_DIRECTORIES ${CMAKE_CURRENT_BINARY_DIR}/EP/Install/glslang/lib)
  list(
    APPEND LLGI_THIRDPARTY_LIBRARIES
    debug glslangd
    debug SPIRVd
    debug OGLCompilerd
    debug OSDependentd
    debug HLSLd
    optimized glslang
    optimized SPIRV
    optimized OGLCompiler
    optimized OSDependent
    optimized HLSL)

  ExternalProject_Add(
      EP_SPIRV-Cross
      SOURCE_DIR      ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/SPIRV-Cross
      PREFIX          ${CMAKE_CURRENT_BINARY_DIR}/EP/Build/SPIRV-Cross
      INSTALL_DIR     ${CMAKE_CURRENT_BINARY_DIR}/EP/Install/SPIRV-Cross
      CMAKE_ARGS      -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR}/EP/Install/SPIRV-Cross
                      -DCMAKE_USER_MAKE_RULES_OVERRIDE=${CMAKE_CURRENT_SOURCE_DIR}/cmake/CFlagOverrides.cmake
                      -DUSE_MSVC_RUNTIME_LIBRARY_DLL=${USE_MSVC_RUNTIME_LIBRARY_DLL}
                      -DCMAKE_DEBUG_POSTFIX=d
                      -DBUILD_SHARED_LIBS=OFF
                      -DSPIRV_CROSS_ENABLE_TESTS=OFF
                      -DSPIRV_CROSS_FORCE_PIC=ON
                      -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
                      -DCMAKE_POSITION_INDEPENDENT_CODE=ON
  )

  list(APPEND LLGI_THIRDPARTY_INCLUDES ${CMAKE_CURRENT_BINARY_DIR}/EP/Install/SPIRV-Cross/include)
  list(APPEND LLGI_THIRDPARTY_LIBRARY_DIRECTORIES ${CMAKE_CURRENT_BINARY_DIR}/EP/Install/SPIRV-Cross/lib)

  list(
    APPEND LLGI_THIRDPARTY_LIBRARIES
    debug spirv-cross-cored
    debug spirv-cross-cd
    debug spirv-cross-cppd
    debug spirv-cross-hlsld
    debug spirv-cross-glsld
    debug spirv-cross-msld
    debug spirv-cross-reflectd
    debug spirv-cross-utild

    optimized spirv-cross-core
    optimized spirv-cross-c
    optimized spirv-cross-cpp
    optimized spirv-cross-hlsl
    optimized spirv-cross-glsl
    optimized spirv-cross-msl
    optimized spirv-cross-reflect
    optimized spirv-cross-util)
endif()

if(BUILD_VULKAN)
  add_definitions(-DENABLE_VULKAN)
  if(BUILD_VULKAN_COMPILER)
    add_definitions(-DENABLE_VULKAN_COMPILER)
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