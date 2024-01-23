#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct PS_INPUT
{
    float4 Position;
    float4 Color;
    float2 UV1;
    float2 UV2;
};

struct main0_out
{
    float4 _entryPointOutput [[color(0)]];
};

struct main0_in
{
    float4 input_Color [[user(locn0)]];
    float2 input_UV1 [[user(locn1)]];
    float2 input_UV2 [[user(locn2)]];
};

static inline __attribute__((always_inline))
float4 _main(thread const PS_INPUT& _input)
{
    float r = length((_input.UV1 - float2(0.5)) * 2.0);
    float a = (r > 1.0) ? 0.0 : ((((0.0 * pow(r, 6.0)) + (1.0 * pow(r, 4.0))) - (2.0 * pow(r, 2.0))) + 1.0);
    float4 c = float4(_input.Color.xyz, _input.Color.w * a);
    return c;
}

fragment main0_out main0(main0_in in [[stage_in]], float4 gl_FragCoord [[position]])
{
    main0_out out = {};
    PS_INPUT _input;
    _input.Position = gl_FragCoord;
    _input.Color = in.input_Color;
    _input.UV1 = in.input_UV1;
    _input.UV2 = in.input_UV2;
    PS_INPUT param = _input;
    out._entryPointOutput = _main(param);
    return out;
}

