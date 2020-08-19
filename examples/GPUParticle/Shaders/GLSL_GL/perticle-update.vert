#version 420

out gl_PerVertex
{
    vec4 gl_Position;
};

struct VS_INPUT
{
    vec3 Position;
    vec2 UV;
};

struct VS_OUTPUT
{
    vec2 UV;
    vec4 Position;
};

layout(location = 0) in vec3 input_Position;
layout(location = 1) in vec2 input_UV;
layout(location = 0) out vec2 _entryPointOutput_UV;

VS_OUTPUT _main(VS_INPUT _input)
{
    VS_OUTPUT _output;
    _output.Position = vec4(_input.Position, 1.0);
    _output.UV = _input.UV;
    return _output;
}

void main()
{
    VS_INPUT _input;
    _input.Position = input_Position;
    _input.UV = input_UV;
    VS_INPUT param = _input;
    VS_OUTPUT flattenTemp = _main(param);
    _entryPointOutput_UV = flattenTemp.UV;
    gl_Position = flattenTemp.Position;
}

