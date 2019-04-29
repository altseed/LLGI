export PATH="$PATH:./"

spirv-cross --msl ../SPIRV/simple_constant_rectangle.vert.spv --output simple_constant_rectangle.vert
spirv-cross --msl ../SPIRV/simple_constant_rectangle.frag.spv --output simple_constant_rectangle.frag

spirv-cross --msl ../SPIRV/simple_texture_rectangle.vert.spv --output simple_texture_rectangle.vert
spirv-cross --msl ../SPIRV/simple_texture_rectangle.frag.spv --output simple_texture_rectangle.frag
