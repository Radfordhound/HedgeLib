struct VS_IN
{
	float3 pos : POSITION;
	//float4 norm : NORMAL;
	float4 col : COLOR;
	//float2 uv0 : TEXCOORD;
};

struct PS_IN
{
	float4 pos : SV_POSITION;
	float4 col : COLOR;
	//float2 uv0 : TEXCOORD;
};

float4x4 worldViewProj;

PS_IN VS(VS_IN input)
{
	PS_IN output = (PS_IN)0;
	float4 pos = float4(input.pos, 1.0f);

	output.pos = mul(pos, worldViewProj);
	output.col = input.col;
	//output.uv0 = input.uv0;

	return output;
}

float4 PS(PS_IN input) : SV_Target
{
	return input.col;
}