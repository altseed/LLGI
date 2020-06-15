glslangValidator ..\GLSL\simple_constant_rectangle.vert -e main -V -l -o simple_constant_rectangle.vert.spv
glslangValidator ..\GLSL\simple_constant_rectangle.frag -e main -V -l -o simple_constant_rectangle.frag.spv

glslangValidator ..\GLSL\simple_texture_rectangle.vert -e main -V -l -o simple_texture_rectangle.vert.spv
glslangValidator ..\GLSL\simple_texture_rectangle.frag -e main -V -l -o simple_texture_rectangle.frag.spv

glslangValidator ../GLSL/simple_mrt_texture_rectangle.frag -e main -V -l -o simple_mrt_texture_rectangle.frag.spv
