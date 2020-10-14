struct PS_IN
{
    float4 pos : SV_POSITION;
    float4 col : COLOR0;
    float2 uv0 : TEXCOORD0;
};

sampler sampler0;
Texture2D texture0;

float4 main(PS_IN input) : SV_Target
{
    float4 col = (input.col * texture0.Sample(sampler0, input.uv0));
    return col;
}
