#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct VS_INPUT
{
    float3 g_position;
    float2 g_uv;
    float4 g_color;
    uint InstanceId;
};

struct VS_OUTPUT
{
    float4 g_position;
    float4 g_color;
};

struct CS_INPUT
{
    float value1;
    float value2;
};

struct read
{
    CS_INPUT _data[1];
};

struct main0_out
{
    float4 _entryPointOutput_g_color [[user(locn0)]];
    float4 gl_Position [[position]];
};

struct main0_in
{
    float3 input_g_position [[attribute(0)]];
    float2 input_g_uv [[attribute(1)]];
    float4 input_g_color [[attribute(2)]];
};

static inline __attribute__((always_inline))
VS_OUTPUT _main(thread const VS_INPUT& _input, const device read& read_1)
{
    VS_OUTPUT _output;
    _output.g_position = float4(_input.g_position, 1.0);
    _output.g_position.x += read_1._data[_input.InstanceId].value1;
    _output.g_position.y += read_1._data[_input.InstanceId].value2;
    _output.g_color = _input.g_color;
    return _output;
}

vertex main0_out main0(main0_in in [[stage_in]], const device read& read_1 [[buffer(10)]], uint gl_InstanceIndex [[instance_id]])
{
    main0_out out = {};
    VS_INPUT _input;
    _input.g_position = in.input_g_position;
    _input.g_uv = in.input_g_uv;
    _input.g_color = in.input_g_color;
    _input.InstanceId = gl_InstanceIndex;
    VS_INPUT param = _input;
    VS_OUTPUT flattenTemp = _main(param, read_1);
    out.gl_Position = flattenTemp.g_position;
    out._entryPointOutput_g_color = flattenTemp.g_color;
    return out;
}

