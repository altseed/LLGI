
file(GLOB files *.h *.cpp)

add_executable(LLGI_Test ${files})

find_library(COCOA_LIBRARY Cocoa)
find_library(APPKIT_LIBRARY AppKit)
find_library(METAL_LIBRARY MetalKit)

set(EXTRA_LIBS ${COCOA_LIBRARY} ${APPKIT_LIBRARY} ${METAL_LIBRARY})
target_link_libraries(LLGI_Test ${EXTRA_LIBS})

target_include_directories(LLGI_Test PUBLIC ../src/)
target_link_libraries(LLGI_Test LLGI)
