
file(GLOB files *.h *.cpp)

add_executable(LLGI_Test ${files})

if(BUILD_METAL)

  find_library(COCOA_LIBRARY Cocoa)
  find_library(METAL_LIBRARY Metal)
  find_library(APPKIT_LIBRARY AppKit)
  find_library(METALKIT_LIBRARY MetalKit)
  find_library(QUARTZ_CORE_LIBRARY QuartzCore)

  set(EXTRA_LIBS ${COCOA_LIBRARY} ${APPKIT_LIBRARY} ${METAL_LIBRARY} ${METALKIT_LIBRARY} ${QUARTZ_CORE_LIBRARY})
  target_link_libraries(LLGI_Test ${EXTRA_LIBS})

endif()

target_include_directories(LLGI_Test PUBLIC ../src/ ${LLGI_GOOGLETEST_INCLUDE})
target_link_directories(LLGI_Test PRIVATE ${LLGI_GOOGLETEST_LIBRARY_DIRECTORY})
target_link_libraries(
	LLGI_Test
  PRIVATE
	LLGI
)

if(MSVC)
  target_link_libraries(
	  LLGI_Test
    PRIVATE
	  debug gtestd
	  optimized gtest
  )
else()
	find_package(Threads REQUIRED)
	target_link_libraries(
    LLGI_Test 
    PRIVATE 
    ${CMAKE_THREAD_LIBS_INIT}
    gtest
    pthread
    X11)
endif()


add_dependencies(LLGI_Test ExternalProject_googletest)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Shaders DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/)
