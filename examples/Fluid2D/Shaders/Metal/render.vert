#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct VS_INPUT
{
    float3 Position;
    float4 Color;
    float2 UV1;
    float2 UV2;
};

struct VS_OUTPUT
{
    float4 Position;
    float4 Color;
    float2 UV1;
    float2 UV2;
};

struct main0_out
{
    float4 _entryPointOutput_Color [[user(locn0)]];
    float2 _entryPointOutput_UV1 [[user(locn1)]];
    float2 _entryPointOutput_UV2 [[user(locn2)]];
    float4 gl_Position [[position]];
};

struct main0_in
{
    float3 input_Position [[attribute(0)]];
    float4 input_Color [[attribute(1)]];
    float2 input_UV1 [[attribute(2)]];
    float2 input_UV2 [[attribute(3)]];
};

static inline __attribute__((always_inline))
VS_OUTPUT _main(thread const VS_INPUT& _input)
{
    VS_OUTPUT _output;
    _output.Position = float4(_input.Position, 1.0);
    _output.UV1 = _input.UV1;
    _output.UV2 = _input.UV2;
    _output.Color = _input.Color;
    return _output;
}

vertex main0_out main0(main0_in in [[stage_in]])
{
    main0_out out = {};
    VS_INPUT _input;
    _input.Position = in.input_Position;
    _input.Color = in.input_Color;
    _input.UV1 = in.input_UV1;
    _input.UV2 = in.input_UV2;
    VS_INPUT param = _input;
    VS_OUTPUT flattenTemp = _main(param);
    out.gl_Position = flattenTemp.Position;
    out._entryPointOutput_Color = flattenTemp.Color;
    out._entryPointOutput_UV1 = flattenTemp.UV1;
    out._entryPointOutput_UV2 = flattenTemp.UV2;
    return out;
}

