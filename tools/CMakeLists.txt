add_subdirectory(ShaderTranspilerCore)
add_subdirectory(ShaderTranspiler)
install(TARGETS ShaderTranspiler DESTINATION ${CMAKE_INSTALL_BINDIR})
