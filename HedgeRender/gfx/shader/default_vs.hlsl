struct VS_IN
{
    float3 pos : POSITION;
    float3 norm : NORMAL;
    float4 col : COLOR;
    float2 uv0 : TEXCOORD0;

    //row_major float4x4 instMatrix : INST_MTX;
};

struct PS_IN
{
    float4 pos : SV_POSITION;
    float4 col : COLOR;
    float2 uv0 : TEXCOORD0;
    float2 uv1 : TEXCOORD1;
};

struct PerFrameData
{
    float4x4 viewProjMtx;
};

struct PerInstanceData
{
    float4x4 worldMtx;
};

ConstantBuffer<PerFrameData> cbPerFrame : register(b0);
ConstantBuffer<PerInstanceData> cbPerInstance : register(b1);

PS_IN main(VS_IN input)
{
    PS_IN output = (PS_IN)0;

    float4 pos = float4(input.pos, 1.0f);
    output.pos = mul(pos, mul(cbPerInstance.worldMtx, cbPerFrame.viewProjMtx));

    output.col = input.col;

    output.uv0 = input.uv0;
    output.uv1 = float2(0.0f, 0.0f);
    //output.uv1 = input.uv1; // TODO

    return output;
}
