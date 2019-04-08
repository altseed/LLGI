#version 440 core
layout(location = 0) in vec3 a_position;
layout(location = 1) in vec2 a_uv;
layout(location = 2) in vec4 a_color;
    
out gl_PerVertex
{
    vec4 gl_Position;
};
    
layout(location = 0) out vec2 v_uv;
layout(location = 1) out vec4 v_color;

void main()
{
    gl_Position.x  = a_position.x;
    gl_Position.y  = a_position.y;
    gl_Position.z  = a_position.z;
    gl_Position.w  = 1.0f;
    v_uv = a_uv;
    v_color = a_color;
}