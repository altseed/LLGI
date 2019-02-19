
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

target_include_directories(LLGI_Test PUBLIC ../src/)
target_link_libraries(LLGI_Test LLGI)
