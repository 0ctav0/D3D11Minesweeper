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
   Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> texture_;
   Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer_;

   bool Init(DeviceManager* d3d, const wchar_t* filename);
   void Begin();
   void End();
   void Draw(const RECT* at, const RECT* tex, const DirectX::XMVECTORF32* color);
   void Draw(const RECT* at, const RECT* tex);
};