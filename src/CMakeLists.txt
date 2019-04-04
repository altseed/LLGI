# The project's name
file(GLOB files *.h *.cpp)
file(GLOB files_pc PC/*.h PC/*.cpp)

list(APPEND files ${files_pc})

if(MSVC)
  file(GLOB files_win Win/*.h Win/*.cpp)
  list(APPEND files ${files_win})
endif()

if(MSVC)
  file(GLOB files_dx12 DX12/*.h DX12/*.cpp)
  list(APPEND files ${files_dx12})
endif()

if(BUILD_VULKAN)
  file(GLOB files_vulkan Vulkan/*.h Vulkan/*.cpp)
  list(APPEND files ${files_vulkan})
  add_definitions(-DENABLE_VULKAN)
endif()

if(BUILD_METAL)
  file(GLOB files_metal Metal/*.h Metal/*.cpp Metal/*.mm)
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

