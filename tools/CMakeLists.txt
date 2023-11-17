if(BUILD_WEBGPU)
  add_definitions(-DENABLE_WEBGPU)
endif()

add_subdirectory(ShaderTranspilerCore)
add_subdirectory(ShaderTranspiler)
install(TARGETS ShaderTranspiler DESTINATION ${CMAKE_INSTALL_BINDIR})
