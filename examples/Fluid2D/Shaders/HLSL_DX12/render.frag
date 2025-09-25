struct PS_INPUT
{
    float4 Position : SV_POSITION;
    float4 Color    : COLOR0;
    float2 UV1 : UV0;
    float2 UV2 : UV1;
};

float4 main(PS_INPUT input) : SV_TARGET
{
    float4 c;
    float r = length((input.UV1 - 0.5) * 2);
    float a = r > 1 ? 0 : (-4 / 9 * pow(r, 6) + 17 / 9 * pow(r, 4) - 22 / 9 * pow(r, 2) + 1);
    c = float4(input.Color.rgb, input.Color.a * a);
    return c;
}