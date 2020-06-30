cmake_minimum_required (VERSION 3.0.0)
project(ShaderTranspiler)

add_executable(
  ShaderTranspiler
  main.cpp
)

target_include_directories(
  ShaderTranspiler
  PUBLIC
  ../ShaderTranspilerCore
)

target_link_libraries(
  ShaderTranspiler
  PUBLIC
  ShaderTranspilerCore
)
