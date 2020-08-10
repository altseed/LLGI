
Texture2D PositionTexture_ : register(t0);
SamplerState PositionSamplerState_ : register(s0);

Texture2D VelocityTexture_ : register(t1);
SamplerState VelocitySamplerState_ : register(s1);

struct PS_INPUT
{
    float2 UV : UV0;
};

struct PS_OUTPUT
{
    float4 Position : SV_TARGET0;
    float4 Velocity : SV_TARGET1;
};

PS_OUTPUT main(PS_INPUT input) : SV_TARGET 
{
    float3 position = PositionTexture_.Sample(PositionSamplerState_, input.UV).xyz;
    float3 velocity = VelocityTexture_.Sample(VelocitySamplerState_, input.UV).xyz;

    position += velocity;

    PS_OUTPUT output;
    output.Position = float4(position, 1);
    output.Velocity = float4(velocity, 1);
    return output;
}
