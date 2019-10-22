cbuffer cbWorld : register(b0)
{
    row_major float4x4 viewProj;
};

cbuffer cbInstance : register(b1)
{
    row_major float4x4 world;
};

Texture2D diffuse : register(t0);
SamplerState diffuse_s : register(s0)
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
    AddressW = Wrap;
};

struct PS_IN
{
    float4 pos : SV_POSITION;
    float4 col : COLOR;
    float2 uv0 : TEXCOORD;
};

float4 main(PS_IN input) : SV_Target
{
    return diffuse.Sample(diffuse_s, input.uv0) *input.col;
}
