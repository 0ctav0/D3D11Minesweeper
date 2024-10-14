cbuffer cb : register(b0)
{
   float width;
   float height;
   float2 reserved;
};
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


float pixelXToNDC(float x)
{
   return 2.0f * x / width - 1;
}

float pixelYToNDC(float y)
{
   return -2.0f * y / height + 1;
}

float NDCXToPixel(float x)
{
   return (x + 1) * width * 0.5f;
}

float NDCYToPixel(float y)
{
   return (1 - y) * height * 0.5f;
}


PS_Input VS_Main(VS_Input vertex)
{
   PS_Input vsOut = (PS_Input) 0;
   float y = NDCYToPixel(vertex.pos.y);
   vsOut.pos = vertex.pos;
   vsOut.tex0 = vertex.tex0;
   if (vertex.pos.x < 0.5)
   {
      //vsOut.color = 1;
   }
   else
   {
      
   }
   vsOut.color = vertex.color;
   return vsOut;
}

static int2 offset = int2(10, 0);


float4 PS_Main(PS_Input frag) : SV_Target
{
   float x = NDCXToPixel(frag.pos.x);
   
   if (frag.tex0.x > 0.5)
   {
      //return 1;
   }
   return tex.Sample(samplerState, frag.tex0);
}