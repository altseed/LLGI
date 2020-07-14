#version 440 core
    
layout(location = 0) in vec2 v_uv;
layout(location = 1) in vec4 v_color;
    
layout(location = 0) out vec4 color;
    
layout(binding = 0, set = 1) uniform Block 
{
	vec4 u_offset;
};

void main()
{
    color = v_color + u_offset;
}
