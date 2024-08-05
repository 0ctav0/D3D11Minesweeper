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
   float2 tex0 : TEXCOORD0;
};

struct PS_Input
{
   float4 pos : SV_POSITION;
   float2 tex0 : TEXCOORD0;
};



PS_Input VS_Main(VS_Input vertex)
{
   PS_Input vsOut = (PS_Input) 0;
   vsOut.pos = vertex.pos;
   vsOut.tex0 = vertex.tex0;
   //float x = vertex.pos.x;
   //float y = vertex.pos.y;
   //float u =  false ? 1 : vertex.tex0.x;
   //float v =  false ? 1 : vertex.tex0.y;
   //vsOut.tex0 = float2(u, v);
   return vsOut;
}

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

float4 PS_Main(PS_Input frag) : SV_Target
{
   float cellMaxU = 64.0f / 480;
   float cellMaxV = 64.0f / 159;
   float cellSize = 32;
   float x = frag.pos.x;
   float y = frag.pos.y;
   float u = frag.tex0.x;
   float v = frag.tex0.y;
   //float offsetX = 63.99;//NDCXToPixel(-0.9);
   float offsetX = NDCXToPixel(-1);
   //float offsetY = 44.79; //NDCYToPixel(0.9);
   float offsetY = pixelYToNDC(128);
   float normX = ((x - offsetX) % cellSize) / cellSize;
   float normY = ((y - offsetY) % cellSize) / cellSize;
   float resultU = normX * cellMaxU;
   float resultV = normY * cellMaxV;
   //if (x > offsetX+320 || y > offsetY+320)
   //{
     //return float4(200, 0, 0, 0.1);
   //}
   return tex.Sample(samplerState, float2(resultU, resultV));
   //return tex.Sample(samplerState, frag.tex0);

}