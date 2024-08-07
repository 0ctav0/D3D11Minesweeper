#pragma once

#include "Types.h"
#include "DeviceManager.h"

using DirectX::XMFLOAT2;

class SpriteRenderer {
private:
   DeviceManager* d3d_;
   VertexPos* spritePtr_;
   float width_, height_;
   unsigned long vertI_;

public:
   Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> textureView_;
   Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer_;

   bool Init(DeviceManager* d3d, const wchar_t* filename);
   void Begin();
   void End();
   void Draw(const RECT1* at, const RECT1* tex, const DirectX::XMVECTORF32* color, Mirror mirror, bool immediate);
   void Draw(const RECT1* at, const RECT1* tex, const DirectX::XMVECTORF32* color);
   void Draw(const RECT1* at, const RECT1* tex, Mirror mirror);
   void Draw(const RECT1* at, const RECT1* tex, bool immediate);
   void Draw(const RECT1* at, const RECT1* tex);
};