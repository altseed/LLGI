cmake_minimum_required (VERSION 3.0.0)
project(ShaderTranspilerCore)

add_library(
  ShaderTranspilerCore
  STATIC
  ShaderTranspilerCore.cpp
  ShaderTranspilerCore.h
  ResourceLimits.cpp
  ResourceLimits.h
)

target_include_directories(ShaderTranspilerCore PUBLIC ${LLGI_THIRDPARTY_INCLUDES})
target_link_libraries(ShaderTranspilerCore PUBLIC ${LLGI_THIRDPARTY_LIBRARIES})
target_link_directories(ShaderTranspilerCore PUBLIC ${LLGI_THIRDPARTY_LIBRARY_DIRECTORIES})

if(WIN32)
# None
else()
  find_package(Threads REQUIRED)
  target_link_libraries(
    ShaderTranspilerCore
    PUBLIC
    ${CMAKE_THREAD_LIBS_INIT}
    pthread)

endif()

if(USE_THIRDPARTY_DIRECTORY)
  add_dependencies(ShaderTranspilerCore ExternalProject_glslang ExternalProject_SPIRV-Cross)
endif()
