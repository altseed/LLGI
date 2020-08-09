add_executable(
  example_GPUParticle
  Main.cpp
)

target_include_directories(
  example_GPUParticle
  PRIVATE
  ../../src/
)

target_link_libraries(
  example_GPUParticle
  PRIVATE
  LLGI
  glfw
)

target_link_directories(example_GPUParticle PRIVATE ${THIRDPARTY_LIBRARY_DIRECTORIES})
target_link_libraries(example_GPUParticle PRIVATE ${THIRDPARTY_LIBRARIES})


if(BUILD_VULKAN)
  find_package(Vulkan REQUIRED)
  target_include_directories(example_GPUParticle PRIVATE ${Vulkan_INCLUDE_DIRS})
  target_link_libraries(example_GPUParticle PRIVATE ${Vulkan_LIBRARIES})

  target_include_directories(example_GPUParticle PRIVATE ${LLGI_THIRDPARTY_INCLUDES})
  target_link_libraries(example_GPUParticle PRIVATE ${LLGI_THIRDPARTY_LIBRARIES})
  target_link_directories(example_GPUParticle PRIVATE ${LLGI_THIRDPARTY_LIBRARY_DIRECTORIES})
endif()
