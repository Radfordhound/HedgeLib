struct PS_IN
{
    float4 pos : SV_POSITION;
    float4 col : COLOR;
    float2 uv0 : TEXCOORD0;
    float2 uv1 : TEXCOORD1;
};

Texture2D diffuse_tex : register(t0);
SamplerState diffuse_sampler : register(s0)
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
    AddressW = Wrap;
};

Texture2D gi_tex : register(t1);
SamplerState gi_sampler : register(s1)
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
    AddressW = Wrap;
};

float4 main(PS_IN input) : SV_Target
{
    // TODO: Make this not literal trash

    return (diffuse_tex.Sample(diffuse_sampler, input.uv0) * input.col);
}
