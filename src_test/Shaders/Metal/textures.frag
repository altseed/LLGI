#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct PS_Input
{
    float4 Pos;
    float2 UV;
    float4 Color;
};

struct main0_out
{
    float4 _entryPointOutput [[color(0)]];
};

struct main0_in
{
    float2 Input_UV [[user(locn0)]];
    float4 Input_Color [[user(locn1)]];
};

static inline __attribute__((always_inline))
float4 _main(PS_Input Input, thread texture2d<float> g_texture1, thread sampler g_sampler1, thread texture2d<float> g_texture4, thread texture2d_array<float> g_texture2, thread sampler g_sampler2, thread texture2d_array<float> g_texture5, thread texture3d<float> g_texture3, thread sampler g_sampler3, thread texture3d<float> g_texture6)
{
    if (Input.UV.x < 0.1500000059604644775390625)
    {
        return g_texture1.sample(g_sampler1, Input.UV);
    }
    else
    {
        if (Input.UV.x < 0.300000011920928955078125)
        {
            return g_texture4.read(uint2(int3(0).xy), int3(0).z) * 0.89999997615814208984375;
        }
        else
        {
            if (Input.UV.x < 0.449999988079071044921875)
            {
                float3 _75 = float3(Input.UV, 1.0);
                return g_texture2.sample(g_sampler2, _75.xy, uint(round(_75.z)));
            }
            else
            {
                if (Input.UV.x < 0.60000002384185791015625)
                {
                    return g_texture5.read(uint2(int4(0, 0, 1, 0).xyz.xy), uint(int4(0, 0, 1, 0).xyz.z), int4(0, 0, 1, 0).w) * 0.89999997615814208984375;
                }
                else
                {
                    if (Input.UV.x < 0.75)
                    {
                        return g_texture3.sample(g_sampler3, float3(Input.UV, 0.5));
                    }
                }
            }
        }
    }
    return g_texture6.read(uint3(int4(0, 0, 1, 0).xyz), int4(0, 0, 1, 0).w) * 0.89999997615814208984375;
}

fragment main0_out main0(main0_in in [[stage_in]], texture2d<float> g_texture1 [[texture(0)]], texture2d_array<float> g_texture2 [[texture(1)]], texture3d<float> g_texture3 [[texture(2)]], texture2d<float> g_texture4 [[texture(3)]], texture2d_array<float> g_texture5 [[texture(4)]], texture3d<float> g_texture6 [[texture(5)]], sampler g_sampler1 [[sampler(0)]], sampler g_sampler2 [[sampler(1)]], sampler g_sampler3 [[sampler(2)]], float4 gl_FragCoord [[position]])
{
    main0_out out = {};
    PS_Input Input;
    Input.Pos = gl_FragCoord;
    Input.UV = in.Input_UV;
    Input.Color = in.Input_Color;
    out._entryPointOutput = _main(Input, g_texture1, g_sampler1, g_texture4, g_texture2, g_sampler2, g_texture5, g_texture3, g_sampler3, g_texture6);
    return out;
}

