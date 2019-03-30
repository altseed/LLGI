# The project's name
file(GLOB files *.h *.cpp)
file(GLOB files_g3 G3/*.h G3/*.cpp)
file(GLOB files_pc G3/PC/*.h G3/PC/*.cpp)

list(APPEND files ${files_pc})
list(APPEND files ${files_g3})

if(MSVC)
  file(GLOB files_win Win/*.h Win/*.cpp)
  list(APPEND files ${files_win})
endif()

if(MSVC)
  file(GLOB files_dx12 G3/DX12/*.h G3/DX12/*.cpp)
  list(APPEND files ${files_dx12})
endif()

if(BUILD_VULKAN)
  file(GLOB files_vulkan Vulkan/*.h Vulkan/*.cpp)
  list(APPEND files ${files_vulkan})
  add_definitions(-DENABLE_VULKAN)
endif()

if(BUILD_METAL)
  file(GLOB files_metal G3/Metal/*.h G3/Metal/*.cpp G3/Metal/*.mm)
  list(APPEND files ${files_metal})
endif()

foreach(FILE ${files}) 
    # Get the directory of the source file
    get_filename_component(PARENT_DIR "${FILE}" DIRECTORY)

    # Remove common directory prefix to make the group
    string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}" "" GROUP "${PARENT_DIR}")

    # Make sure we are using windows slashes
    string(REPLACE "/" "\\" GROUP "${GROUP}")

    # Group into "Source Files" and "Header Files"
    if ("${FILE}" MATCHES ".*\\.cpp")
       set(GROUP "Files${GROUP}")
    elseif("${FILE}" MATCHES ".*\\.c")
       set(GROUP "Files${GROUP}")
    elseif("${FILE}" MATCHES ".*\\.cxx")
       set(GROUP "Files${GROUP}")
    elseif("${FILE}" MATCHES ".*\\.h")
       set(GROUP "Files${GROUP}")
    elseif("${FILE}" MATCHES ".*\\.mm")
        set(GROUP "Files${GROUP}")
    endif()

    source_group("${GROUP}" FILES "${FILE}")
endforeach()

add_library(LLGI STATIC ${files})

