cmake_minimum_required (VERSION 3.0.0)
project(ShaderTranspilerCore)

add_library(
  ShaderTranspilerCore
  STATIC
  ShaderTranspilerCore.cpp
  ShaderTranspilerCore.h
)

target_include_directories(
  ShaderTranspilerCore
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/../../thirdparty/glslang/
  ${CMAKE_CURRENT_SOURCE_DIR}/../../thirdparty/glslang/glslang
  ${CMAKE_CURRENT_SOURCE_DIR}/../../thirdparty/SPIRV-Cross
  ${CMAKE_CURRENT_SOURCE_DIR}/../../thirdparty/SPIRV-Cross/include
)

target_link_libraries(
  ShaderTranspilerCore
  PUBLIC
  spirv-cross-core
  spirv-cross-c
  spirv-cross-cpp
  spirv-cross-hlsl
  spirv-cross-glsl
  spirv-cross-msl
  spirv-cross-reflect
  spirv-cross-util
  glslang
  SPIRV
  HLSL
  OGLCompiler
  OSDependent
  SPVRemapper
  glslang-default-resource-limits
)