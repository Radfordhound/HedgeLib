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

cbuffer VS_CONSTANT_BUFFER : register(b0)
{
	float4x4 worldViewProj;
};

Texture2D tex0 : register(t0);
SamplerState Sampler : register(s0)
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

	output.pos = mul(pos, worldViewProj);
	output.col = input.col;
	output.uv0 = input.uv0;

	return output;
}

float4 PS(PS_IN input) : SV_Target
{
	//return input.col;
	return tex0.Sample(Sampler, input.uv0) * input.col;
}