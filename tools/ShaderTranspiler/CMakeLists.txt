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

target_include_directories(ShaderTranspiler PRIVATE ${LLGI_THIRDPARTY_INCLUDES})
target_link_libraries(ShaderTranspiler PRIVATE ${LLGI_THIRDPARTY_LIBRARIES})
target_link_directories(ShaderTranspiler PRIVATE ${LLGI_THIRDPARTY_LIBRARY_DIRECTORIES})

if(WIN32)
# None
else()
  find_package(Threads REQUIRED)
  target_link_libraries(
    ShaderTranspiler 
    PRIVATE 
    ${CMAKE_THREAD_LIBS_INIT}
    pthread)

endif()