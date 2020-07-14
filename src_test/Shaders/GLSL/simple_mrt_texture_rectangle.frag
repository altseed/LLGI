
#version 440 core

layout(location = 0) in vec2 v_uv;
layout(location = 1) in vec4 v_color;

// 0 is constant buffer
layout(binding = 1) uniform sampler2D mainTexture;

layout(location = 0) out vec4 color0;
layout(location = 1) out vec4 color1;

void main()
{
    vec4 c = texture(mainTexture, v_uv);
    c.a = 1.0f;
    color0 = c;

	c.r = 1.0f - c.r;
	c.g = 1.0f - c.g;
	c.b = 1.0f - c.b;

    color1 = c;
}
