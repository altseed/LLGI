
add_executable(
    example_glfw
    main.cpp)

target_include_directories(
    example_glfw
	PRIVATE
	../../src/
)

target_link_libraries(
    example_glfw
    PRIVATE
	LLGI
	glfw
)
