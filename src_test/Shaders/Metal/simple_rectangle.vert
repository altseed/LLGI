struct VertexIn {
    metal::float3 position [[attribute(0)]];
    metal::float2 uv [[attribute(1)]];
    metal::float4 color [[attribute(2)]];
};

struct VertexOut {
    metal::float4 position [[position]];
    metal::float2 uv;
    metal::float4 color;
};

vertex VertexOut main0(VertexIn vertex_array [[stage_in]], unsigned int vid [[vertex_id]]) {
    
    VertexOut vo;
    vo.position = metal::float4(vertex_array.position, 1.0);
    vo.color = (metal::float4)vertex_array.color;
    return vo;
}
    