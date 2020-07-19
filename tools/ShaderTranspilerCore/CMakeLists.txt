cmake_minimum_required(VERSION 3.0.0)
project(ShaderTranspilerCore)

add_library(
  ShaderTranspilerCore STATIC ShaderTranspilerCore.cpp ShaderTranspilerCore.h
                              ResourceLimits.cpp ResourceLimits.h)

target_compile_features(ShaderTranspilerCore PUBLIC cxx_std_11)
target_include_directories(ShaderTranspilerCore
                           PUBLIC ${LLGI_THIRDPARTY_INCLUDES})

if(USE_THIRDPARTY_DIRECTORY)
  target_link_libraries(ShaderTranspilerCore
                        PUBLIC ${LLGI_THIRDPARTY_LIBRARIES})
  target_link_directories(ShaderTranspilerCore PUBLIC
                          ${LLGI_THIRDPARTY_LIBRARY_DIRECTORIES})
endif()

if(WIN32)
  # None
else()
  find_package(Threads REQUIRED)
  target_link_libraries(ShaderTranspilerCore PUBLIC ${CMAKE_THREAD_LIBS_INIT}
                                                    pthread)

endif()

if(USE_THIRDPARTY_DIRECTORY)
  add_dependencies(ShaderTranspilerCore EP_glslang EP_SPIRV-Cross)
endif()
