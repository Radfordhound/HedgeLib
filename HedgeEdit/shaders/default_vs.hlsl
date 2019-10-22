cbuffer cbWorld : register(b0)
{
    row_major float4x4 viewProj;
};

cbuffer cbInstance : register(b1)
{
    row_major float4x4 world;
};

struct VS_IN
{
    float3 pos : POSITION;
    float3 norm : NORMAL;
    float4 col : COLOR;
    float2 uv0 : TEXCOORD;
};

struct PS_IN
{
    float4 pos : SV_POSITION;
    float4 col : COLOR;
    float2 uv0 : TEXCOORD;
};

PS_IN main(VS_IN input)
{
    PS_IN output = (PS_IN)0;
    float4 pos = float4(input.pos, 1.0f);

    output.pos = mul(pos, mul(world, viewProj));
    output.col = input.col;
    output.uv0 = input.uv0;

    return output;
}
