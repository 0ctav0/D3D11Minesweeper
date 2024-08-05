Texture2D tex : register(t0);
sampler samplerState : register(s0);

struct VS_Input
{
   float4 pos : SV_POSITION;
   float4 color : COLOR0;
   float2 tex0 : TEXCOORD0;
};

struct PS_Input
{
   float4 pos : SV_POSITION;
   float4 color : COLOR0;
   float2 tex0 : TEXCOORD0;
};


PS_Input VS_Main(VS_Input vertex)
{
   PS_Input vsOut = (PS_Input) 0;
   vsOut.pos = vertex.pos;
   vsOut.tex0 = vertex.tex0;
   vsOut.color = vertex.color;
   return vsOut;
}

float4 PS_Main(PS_Input frag) : SV_Target
{
   return tex.Sample(samplerState, frag.tex0) * frag.color;
}