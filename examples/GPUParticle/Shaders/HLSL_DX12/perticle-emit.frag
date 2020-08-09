
struct PS_INPUT
{
    float4 PositionData : UV0;
    float4 VelocityAndLifeTimeData : COLOR0;
};

struct VS_OUTPUT
{
    float4 Position : SV_TARGET0;
    float4 VelocityAndLifeTime : SV_TARGET1;
};

VS_OUTPUT main(PS_INPUT input)
{
    VS_OUTPUT output;
    output.Position = input.PositionData;//float4(1, 0, 0, 1);//
    output.VelocityAndLifeTime = input.VelocityAndLifeTimeData;// float4(0, 1, 0, 1);//
    return output;
}
