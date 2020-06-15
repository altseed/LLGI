#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 color1 [[color(0)]];
    float4 color2 [[color(1)]];
};

struct main0_in
{
    float2 v_uv [[user(locn0)]];
    float4 v_color [[user(locn1)]];
};

fragment main0_out main0(main0_in in [[stage_in]], texture2d<float> mainTexture [[texture(0)]], sampler mainTextureSmplr [[sampler(0)]])
{
    main0_out out = {};
    float4 c = mainTexture.sample(mainTextureSmplr, in.v_uv);
    c.w = 1.0f;
    out.color1 = c;

    c.x = 1.0f - c.x;
    c.y = 1.0f - c.y;
    c.z = 1.0f - c.z;

    out.color2 = c;

    return out;
}

