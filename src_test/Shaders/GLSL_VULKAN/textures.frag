#version 430

struct PS_Input
{
    vec4 Pos;
    vec2 UV;
    vec4 Color;
};

layout(location = 0, set = 1, binding = 1) uniform sampler2D Sampler_g_sampler1;
layout(location = 3, set = 1, binding = 4) uniform sampler2D Sampler_g_sampler4;
layout(location = 1, set = 1, binding = 2) uniform sampler2DArray Sampler_g_sampler2;
layout(location = 4, set = 1, binding = 5) uniform sampler2DArray Sampler_g_sampler5;
layout(location = 2, set = 1, binding = 3) uniform sampler3D Sampler_g_sampler3;
layout(location = 5, set = 1, binding = 6) uniform sampler3D Sampler_g_sampler6;

layout(location = 0) in vec2 Input_UV;
layout(location = 1) in vec4 Input_Color;
layout(location = 0) out vec4 _entryPointOutput;

vec4 _main(PS_Input Input)
{
    if (Input.UV.x < 0.1500000059604644775390625)
    {
        return texture(Sampler_g_sampler1, Input.UV);
    }
    else
    {
        if (Input.UV.x < 0.300000011920928955078125)
        {
            return texelFetch(sampler2D(g_texture4, SPIRV_Cross_DummySampler), ivec3(0).xy, ivec3(0).z) * 0.89999997615814208984375;
        }
        else
        {
            if (Input.UV.x < 0.449999988079071044921875)
            {
                return texture(Sampler_g_sampler2, vec3(Input.UV, 1.0));
            }
            else
            {
                if (Input.UV.x < 0.60000002384185791015625)
                {
                    return texelFetch(sampler2DArray(g_texture5, SPIRV_Cross_DummySampler), ivec4(0, 0, 1, 0).xyz, ivec4(0, 0, 1, 0).w) * 0.89999997615814208984375;
                }
                else
                {
                    if (Input.UV.x < 0.75)
                    {
                        return texture(Sampler_g_sampler3, vec3(Input.UV, 0.5));
                    }
                }
            }
        }
    }
    return texelFetch(sampler3D(g_texture6, SPIRV_Cross_DummySampler), ivec4(0, 0, 1, 0).xyz, ivec4(0, 0, 1, 0).w) * 0.89999997615814208984375;
}

void main()
{
    PS_Input Input;
    Input.Pos = gl_FragCoord;
    Input.UV = Input_UV;
    Input.Color = Input_Color;
    _entryPointOutput = _main(Input);
}

