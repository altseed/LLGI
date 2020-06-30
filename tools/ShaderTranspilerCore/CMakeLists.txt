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

target_include_directories(ShaderTranspilerCore PRIVATE ${LLGI_THIRDPARTY_INCLUDES})
target_link_libraries(ShaderTranspilerCore PRIVATE ${LLGI_THIRDPARTY_LIBRARIES})
target_link_directories(ShaderTranspilerCore PRIVATE ${LLGI_THIRDPARTY_LIBRARY_DIRECTORIES})

add_dependencies(ShaderTranspilerCore ExternalProject_glslang ExternalProject_SPIRV-Cross)