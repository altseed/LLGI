#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct Block
{
    float4 u_offset;
};

struct main0_out
{
    float2 v_uv [[user(locn0)]];
    float4 v_color [[user(locn1)]];
    float4 gl_Position [[position]];
};

struct main0_in
{
    float3 a_position [[attribute(0)]];
    float2 a_uv [[attribute(1)]];
    float4 a_color [[attribute(2)]];
};

vertex main0_out main0(main0_in in [[stage_in]], constant Block& _39 [[buffer(0)]])
{
    main0_out out = {};
    out.gl_Position.x = in.a_position.x;
    out.gl_Position.y = in.a_position.y;
    out.gl_Position.z = in.a_position.z;
    out.gl_Position.w = 1.0;
    out.gl_Position += _39.u_offset;
    out.v_uv = in.a_uv;
    out.v_color = in.a_color;
    return out;
}

