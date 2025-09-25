#version 430

struct PS_INPUT
{
    vec4 Position;
    vec4 Color;
    vec2 UV1;
    vec2 UV2;
};

layout(location = 0) in vec4 input_Color;
layout(location = 1) in vec2 input_UV1;
layout(location = 2) in vec2 input_UV2;
layout(location = 0) out vec4 _entryPointOutput;

vec4 _main(PS_INPUT _input)
{
    float r = length((_input.UV1 - vec2(0.5)) * 2.0);
    float a = (r > 1.0) ? 0.0 : ((((0.0 * pow(r, 6.0)) + (1.0 * pow(r, 4.0))) - (2.0 * pow(r, 2.0))) + 1.0);
    vec4 c = vec4(_input.Color.xyz, _input.Color.w * a);
    return c;
}

void main()
{
    PS_INPUT _input;
    _input.Position = gl_FragCoord;
    _input.Color = input_Color;
    _input.UV1 = input_UV1;
    _input.UV2 = input_UV2;
    PS_INPUT param = _input;
    _entryPointOutput = _main(param);
}

