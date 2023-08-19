struct VS_INPUT{
    float3 Position : POSITION0;
    float4 Color : COLOR0;
    float2 UV1 : UV0;
    float2 UV2 : UV1;
};

struct VS_OUTPUT{
    float4 Position : SV_POSITION;
    float4 Color    : COLOR0;
    float2 UV1 : UV0;
    float2 UV2 : UV1;
};

VS_OUTPUT main(VS_INPUT input){
    VS_OUTPUT output;
    output.Position = float4(input.Position / 200.0f - float3(0.5f, 0.5f, 0), 1.0f);
    // output.Position = float4(input.Position, 1.0f);
    output.UV1 = input.UV1;
    output.UV2 = input.UV2;
    output.Color = input.Color;
    return output;
}