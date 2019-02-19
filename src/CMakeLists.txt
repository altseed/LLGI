# The project's name
project(LLGI)

file(GLOB files G3/*.h G3/*.cpp)
file(GLOB files_pc G3/PC/*.h G3/PC/*.cpp)

if(BUILD_VULKAN)

file(GLOB files_vulkan G3/Vulkan/*.h G3/Vulkan/*.cpp)
add_library(LLGI STATIC ${files} ${files_vulkan} ${files_pc})

endif()

if(BUILD_METAL)

file(GLOB files_metal G3/Metal/*.h G3/Metal/*.cpp G3/Metal/*.mm)
add_library(LLGI STATIC ${files} ${files_metal} ${files_pc})

endif()