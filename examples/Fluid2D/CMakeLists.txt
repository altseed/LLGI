add_executable(
  example_Fluid2D
  Fluid2D.h
  Fluid2D.cpp
  Main.cpp
)

target_include_directories(
  example_Fluid2D
  PRIVATE
  ../../src/
)

target_link_libraries(
  example_Fluid2D
  PRIVATE
  LLGI
  glfw
)

target_link_directories(example_Fluid2D PRIVATE ${THIRDPARTY_LIBRARY_DIRECTORIES})
target_link_libraries(example_Fluid2D PRIVATE ${THIRDPARTY_LIBRARIES})
target_compile_definitions(example_Fluid2D PRIVATE EXAMPLE_ASSET_DIR="${CMAKE_CURRENT_LIST_DIR}")

if(WIN32)
  # None
elseif(APPLE)

  find_library(COCOA_LIBRARY Cocoa)
  find_library(METAL_LIBRARY Metal)
  find_library(APPKIT_LIBRARY AppKit)
  find_library(METALKIT_LIBRARY MetalKit)
  find_library(QUARTZ_CORE_LIBRARY QuartzCore)

  set(EXTRA_LIBS ${COCOA_LIBRARY} ${APPKIT_LIBRARY} ${METAL_LIBRARY} ${METALKIT_LIBRARY} ${QUARTZ_CORE_LIBRARY})
  target_link_libraries(example_Fluid2D PRIVATE ${EXTRA_LIBS})

else()

  find_package(Threads REQUIRED)
  target_link_libraries(
    example_Fluid2D
    PRIVATE
    ${CMAKE_THREAD_LIBS_INIT}
    pthread
    X11
    X11-xcb
  )

endif()


if(BUILD_VULKAN)
  find_package(Vulkan REQUIRED)
  target_include_directories(example_Fluid2D PRIVATE ${Vulkan_INCLUDE_DIRS})
  target_link_libraries(example_Fluid2D PRIVATE ${Vulkan_LIBRARIES})

  target_include_directories(example_Fluid2D PRIVATE ${LLGI_THIRDPARTY_INCLUDES})
  target_link_libraries(example_Fluid2D PRIVATE ${LLGI_THIRDPARTY_LIBRARIES})
  target_link_directories(example_Fluid2D PRIVATE ${LLGI_THIRDPARTY_LIBRARY_DIRECTORIES})
endif()
