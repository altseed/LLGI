
Texture2D g_texture1 : register(t0);
SamplerState g_sampler1 : register(s0);

Texture2DArray g_texture2 : register(t1);
SamplerState g_sampler2 : register(s1);

Texture3D g_texture3 : register(t2);
SamplerState g_sampler3 : register(s2);

Texture2D g_texture4 : register(t3);
SamplerState g_sampler4 : register(s3);

Texture2DArray g_texture5 : register(t4);
SamplerState g_sampler5 : register(s4);

Texture3D g_texture6 : register(t5);
SamplerState g_sampler6 : register(s5);

struct PS_Input
{
	float4 Pos : SV_POSITION;
	float2 UV : UV0;
	float4 Color : COLOR0;
};

float4 main(const PS_Input Input): SV_Target
{
	if(Input.UV.x < 0.15)
	{
		return g_texture1.Sample(g_sampler1, Input.UV);
	}
	else if (Input.UV.x < 0.30)
	{
		return g_texture4.Load(int3(0,0,0)) * 0.9;
	}
	else if(Input.UV.x < 0.45)
	{
		return g_texture2.Sample(g_sampler2, float3(Input.UV, 1));
	}
	else if (Input.UV.x < 0.60)
	{
		return g_texture5.Load(int4(0, 0, 1, 0)) * 0.9;
	}
	else if (Input.UV.x < 0.75)
	{
		return g_texture3.Sample(g_sampler3, float3(Input.UV, 0.5f));
	}
	return g_texture6.Load(int4(0, 0, 1, 0)) * 0.9;
}
