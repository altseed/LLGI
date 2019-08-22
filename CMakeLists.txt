cmake_minimum_required(VERSION 3.12)

# The project's name
project(LLGI)

include(ExternalProject)
set (CMAKE_CXX_STANDARD 11)

option(BUILD_METAL "build metal" OFF)
option(BUILD_VULKAN "build vulkan" OFF)
option(BUILD_TEST "build test" OFF)

option(USE_MSVC_RUNTIME_LIBRARY_DLL "compile as multithreaded DLL" ON)

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
endif()

if(BUILD_METAL)
  add_definitions(-DENABLE_METAL)
endif()

add_subdirectory("src")

if(BUILD_TEST)
  ExternalProject_Add(
    ExternalProject_googletest
    GIT_REPOSITORY    https://github.com/google/googletest.git
    GIT_TAG           release-1.8.1
    SOURCE_DIR        ${CMAKE_CURRENT_BINARY_DIR}/ThirdParty/Source/googletest
    BINARY_DIR        ${CMAKE_CURRENT_BINARY_DIR}/ThirdParty/Build/googletest
    PREFIX            ${CMAKE_CURRENT_BINARY_DIR}/ThirdParty/Build/googletest
    INSTALL_DIR       ${CMAKE_CURRENT_BINARY_DIR}/ThirdParty/Install/googletest
    CMAKE_ARGS        -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR}/ThirdParty/Install/googletest
                      -Dgtest_force_shared_crt=${USE_MSVC_RUNTIME_LIBRARY_DLL}
  )
  set(LLGI_GOOGLETEST_INCLUDE ${CMAKE_CURRENT_BINARY_DIR}/ThirdParty/Install/googletest/include)
  set(LLGI_GOOGLETEST_LIBRARY_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/ThirdParty/Install/googletest/lib)
  add_subdirectory("src_test")
endif()
