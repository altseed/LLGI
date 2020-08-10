
Texture2D PositionTexture_ : register(t0);
SamplerState PositionSamplerState_ : register(s0);

Texture2D VelocityTexture_ : register(t1);
SamplerState VelocitySamplerState_ : register(s1);

struct VS_INPUT
{
    float3 Position : POSITION0;
    float2 UV : UV0;
    uint InstanceId : SV_InstanceID;    // = ParticleId
};

struct VS_OUTPUT
{
    float2 UV : UV0;
    float4 Position : SV_POSITION;
};

cbuffer GPUParticleTextureInfo : register(b0)
{
    float4 TextureResolution;
    float4x4 ViewProjMatrix;
};

VS_OUTPUT main(VS_INPUT input)
{
    float2 texelPos = float2(fmod(input.InstanceId, TextureResolution.x), input.InstanceId / TextureResolution.x);
    float2 fetchUV = texelPos * TextureResolution.zw;
    float3 posOffset = PositionTexture_.SampleLevel(PositionSamplerState_, fetchUV, 0).xyz;

    float3 worldPos = input.Position + posOffset;

    VS_OUTPUT output;
    output.Position = mul(ViewProjMatrix, float4(worldPos, 1.0f));
    //output.Position = float4(worldPos, 1.0f);
    output.UV = input.UV;

    return output;
}
