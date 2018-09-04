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

cbuffer cbDefault : register(b0)
{
	row_major float4x4 viewProj;
};

cbuffer cbDefaultInstance : register(b1)
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

PS_IN VS(VS_IN input)
{
	PS_IN output = (PS_IN)0;
	float4 pos = float4(input.pos, 1.0f);

	output.pos = mul(pos, mul(world, viewProj));
	output.col = input.col;
	output.uv0 = input.uv0;

	return output;
}

float4 PS(PS_IN input) : SV_Target
{
	return diffuse.Sample(diffuse_s, input.uv0) * input.col;
}