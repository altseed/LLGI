#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct Block
{
    float4 u_offset;
};

struct main0_out
{
    float4 color [[color(0)]];
};

struct main0_in
{
    float4 v_color [[user(locn1)]];
};

fragment main0_out main0(main0_in in [[stage_in]], constant Block& _15 [[buffer(0)]])
{
    main0_out out = {};
    out.color = in.v_color + _15.u_offset;
    return out;
}

