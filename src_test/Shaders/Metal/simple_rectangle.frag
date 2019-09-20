struct VertexOut {
    metal::float4 position [[position]];
    metal::float2 uv;
    metal::float4 color;
};

fragment metal::half4 main0(VertexOut input [[stage_in]]) {
    return metal::half4(input.color);
}