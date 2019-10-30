
if(WIN32)
add_executable(
    example_imgui
    main.cpp
    ../thirdparty/imgui/imgui.cpp
    ../thirdparty/imgui/imgui.h
    ../thirdparty/imgui/imgui_draw.cpp
    ../thirdparty/imgui/imgui_widgets.cpp
    ../thirdparty/imgui/imgui_impl_glfw.cpp
    ../thirdparty/imgui/imgui_impl_dx12.cpp
)
endif()

target_include_directories(
    example_imgui
    PRIVATE
    ../../src/
    ../thirdparty/imgui/
)

target_link_libraries(
    example_imgui
    PRIVATE
    LLGI
    glfw
)

if(WIN32)
# None
elseif(APPLE)

    find_library(COCOA_LIBRARY Cocoa)
    find_library(METAL_LIBRARY Metal)
    find_library(APPKIT_LIBRARY AppKit)
    find_library(METALKIT_LIBRARY MetalKit)
    find_library(QUARTZ_CORE_LIBRARY QuartzCore)

    set(EXTRA_LIBS ${COCOA_LIBRARY} ${APPKIT_LIBRARY} ${METAL_LIBRARY} ${METALKIT_LIBRARY} ${QUARTZ_CORE_LIBRARY})
    target_link_libraries(example_glfw PRIVATE ${EXTRA_LIBS})

else()

    find_package(Threads REQUIRED)
    target_link_libraries(
    example_glfw 
    PRIVATE 
    ${CMAKE_THREAD_LIBS_INIT}
    pthread
    X11
    X11-xcb)

endif()
