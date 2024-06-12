#version 430

struct VS_INPUT
{
    vec3 Position;
    vec4 Color;
    vec2 UV1;
    vec2 UV2;
};

struct VS_OUTPUT
{
    vec4 Position;
    vec4 Color;
    vec2 UV1;
    vec2 UV2;
};

layout(location = 0) in vec3 input_Position;
layout(location = 1) in vec4 input_Color;
layout(location = 2) in vec2 input_UV1;
layout(location = 3) in vec2 input_UV2;
layout(location = 0) out vec4 _entryPointOutput_Color;
layout(location = 1) out vec2 _entryPointOutput_UV1;
layout(location = 2) out vec2 _entryPointOutput_UV2;

VS_OUTPUT _main(VS_INPUT _input)
{
    VS_OUTPUT _output;
    _output.Position = vec4(_input.Position, 1.0);
    _output.UV1 = _input.UV1;
    _output.UV2 = _input.UV2;
    _output.Color = _input.Color;
    return _output;
}

void main()
{
    VS_INPUT _input;
    _input.Position = input_Position;
    _input.Color = input_Color;
    _input.UV1 = input_UV1;
    _input.UV2 = input_UV2;
    VS_INPUT param = _input;
    VS_OUTPUT flattenTemp = _main(param);
    vec4 _position = flattenTemp.Position;
    _position.y = -_position.y;
    gl_Position = _position;
    _entryPointOutput_Color = flattenTemp.Color;
    _entryPointOutput_UV1 = flattenTemp.UV1;
    _entryPointOutput_UV2 = flattenTemp.UV2;
}

