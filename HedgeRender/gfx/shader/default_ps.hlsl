struct PS_IN
{
    float4 pos : SV_POSITION;
    float4 col : COLOR;
    float2 uv0 : TEXCOORD0;
    float2 uv1 : TEXCOORD1;
};

//Texture2D diffuse_tex : register(t0);
//SamplerState diffuse_sampler : register(s0);

//Texture2D gi_tex : register(t1);
//SamplerState gi_sampler : register(s1);

float4 main(PS_IN input) : SV_Target
{
    // TODO: Make this not literal trash

    return input.col;
    //return (diffuse_tex.Sample(diffuse_sampler, input.uv0) * input.col);

    /* gi_tex.Sample(gi_sampler, input.uv1) */
    /*float4 difCol = diffuse_tex.Sample(diffuse_sampler, input.uv0);
    float4 giCol = normalize(gi_tex.Sample(gi_sampler, input.uv1));

    return (float4(difCol.x * giCol.x, difCol.y * giCol.y, difCol.z * giCol.z, difCol.w) * input.col) * 2;*/
}
