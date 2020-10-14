struct VS_IN
{
    float2 pos : POSITION;
    float2 uv0 : TEXCOORD;
    float4 col : COLOR;
};

struct PS_IN
{
    float4 pos : SV_POSITION;
    float4 col : COLOR0;
    float2 uv0 : TEXCOORD0;
};

cbuffer cbDefault : register(b0)
{
    float4x4 proj;
};

PS_IN main(VS_IN input)
{
    PS_IN output = (PS_IN)0;

    output.pos = mul(proj, float4(input.pos.xy, 0.0f, 1.0f));
    output.col = input.col;
    output.uv0 = input.uv0;

    return output;
}
