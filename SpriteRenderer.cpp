#include "pch.h"
#include "Types.h"
#include "DeviceManager.h"
#include "SpriteRenderer.h"

bool SpriteRenderer::Init(DeviceManager* d3d, const wchar_t* filename) {
   d3d_ = d3d;
   Microsoft::WRL::ComPtr<ID3D11Resource> resourceAtlas;
   DX::ThrowIfFailed(DirectX::CreateWICTextureFromFile(d3d_->device_.Get(),
      filename, resourceAtlas.GetAddressOf(), atlasView_.ReleaseAndGetAddressOf()), "Failed to create a texture atlas from a file");

   Microsoft::WRL::ComPtr<ID3D11Resource> resourceBackgroundCell;
   DX::ThrowIfFailed(DirectX::CreateWICTextureFromFile(d3d_->device_.Get(),
      L"img/background-cell.png", resourceBackgroundCell.GetAddressOf(), backgroundCellView_.ReleaseAndGetAddressOf()), "Failed to create a background texture from a file");
   //memcpy(resourceBackgroundCell.GetAddressOf(), resourceAtlas.Get(), sizeof(resourceAtlas));

   D3D11_BOX sourceRegion;
   sourceRegion.left = 5;
   sourceRegion.right = 54 + 5;
   sourceRegion.top = 5;
   sourceRegion.bottom = 54 + 5;
   sourceRegion.front = 0;
   sourceRegion.back = 1;
   //d3d_->ctx_->CopySubresourceRegion(resourceBackgroundCell.Get(), 0, 0, 0, 0, resourceAtlas.Get(), 0, &sourceRegion);

   Microsoft::WRL::ComPtr<ID3D11Texture2D> tex;
   DX::ThrowIfFailed(resourceAtlas.As(&tex), "Failed resource as tex");

   CD3D11_TEXTURE2D_DESC desc;
   tex->GetDesc(&desc);

   width_ = desc.Width;
   height_ = desc.Height;

   // setting up vertex buffer
   D3D11_BUFFER_DESC vertexDesc = {};
   vertexDesc.Usage = D3D11_USAGE_DYNAMIC;
   vertexDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
   vertexDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
   vertexDesc.ByteWidth = sizeof(VertexPos) * 4 * 10'000; // for 10,000 square meshes
   DX::ThrowIfFailed(d3d_->device_->CreateBuffer(&vertexDesc, 0, vertexBuffer_.GetAddressOf()), "Failed to create the vertex buffer");

   return true;
}

void SpriteRenderer::Begin() {
   vertI_ = 0;
   D3D11_MAPPED_SUBRESOURCE mapResource;
   DX::ThrowIfFailed(d3d_->ctx_->Map(vertexBuffer_.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0,
      &mapResource), "Failed to map resource");

   spritePtr_ = (VertexPos*)mapResource.pData;
}

void SpriteRenderer::End() {
   d3d_->ctx_->Unmap(vertexBuffer_.Get(), 0);
   d3d_->ctx_->Draw(vertI_, 0);
}

void SpriteRenderer::Draw(const RECT1* at, const RECT1* tex, const DirectX::XMVECTORF32* color, Mirror mirror, bool immediate) {
   auto left = tex->left / width_;
   auto right = tex->right / width_;
   auto top = tex->top / height_;
   auto bottom = tex->bottom / height_;

   auto trcTex = XMFLOAT2(
      mirror & Mirror::FlipHorizontally ? left : right,
      mirror & Mirror::FlipVertically ? bottom : top);
   auto brcTex = XMFLOAT2(
      mirror & Mirror::FlipHorizontally ? left : right,
      mirror & Mirror::FlipVertically ? top : bottom);
   auto tlcTex = XMFLOAT2(
      mirror & Mirror::FlipHorizontally ? right : left,
      mirror & Mirror::FlipVertically ? bottom : top);
   auto blcTex = XMFLOAT2(
      mirror & Mirror::FlipHorizontally ? right : left,
      mirror & Mirror::FlipVertically ? top : bottom);

   if (immediate) Begin();

   spritePtr_[vertI_].pos = d3d_->PixelXMFLOAT3(at->right, at->top); // trc
   DirectX::XMStoreFloat4(&spritePtr_[vertI_].color, *color);
   spritePtr_[vertI_].tex0 = trcTex;
   vertI_++;

   spritePtr_[vertI_].pos = d3d_->PixelXMFLOAT3(at->right, at->bottom); // brc
   DirectX::XMStoreFloat4(&spritePtr_[vertI_].color, *color);
   spritePtr_[vertI_].tex0 = brcTex;
   vertI_++;

   spritePtr_[vertI_].pos = d3d_->PixelXMFLOAT3(at->left, at->top); // tlc
   DirectX::XMStoreFloat4(&spritePtr_[vertI_].color, *color);
   spritePtr_[vertI_].tex0 = tlcTex;
   vertI_++;

   spritePtr_[vertI_].pos = d3d_->PixelXMFLOAT3(at->left, at->bottom); // blc
   DirectX::XMStoreFloat4(&spritePtr_[vertI_].color, *color);
   spritePtr_[vertI_].tex0 = blcTex;
   vertI_++;

   if (immediate) End();
}

void SpriteRenderer::Draw(const RECT1* at, const RECT1* tex, const DirectX::XMVECTORF32* color) {
   Draw(at, tex, color, Mirror::None, false);
}


void SpriteRenderer::Draw(const RECT1* at, const RECT1* tex, Mirror mirror) {
   Draw(at, tex, &DirectX::Colors::White, mirror, false);
}

void SpriteRenderer::Draw(const RECT1* at, const RECT1* tex, bool immediate) {
   Draw(at, tex, &DirectX::Colors::White, Mirror::None, immediate);
}

void SpriteRenderer::Draw(const RECT1* at, const RECT1* tex) {
   Draw(at, tex, &DirectX::Colors::White, Mirror::None, false);
}

void SpriteRenderer::DrawBackgroundCell(RECT1* at)
{
   auto texWidth = 54.f;
   auto texHeight = 54.f;
   auto timesX = at->Width() / texWidth;
   auto timesY = at->Height() / texHeight;
   auto left = 0;
   auto right = timesX;
   auto top = 0;
   auto bottom = timesY;

   auto trcTex = XMFLOAT2(
      right,
      top);
   auto brcTex = XMFLOAT2(
      right,
      bottom);
   auto tlcTex = XMFLOAT2(
      left,
      top);
   auto blcTex = XMFLOAT2(
      left,
      bottom);

   Begin();

   spritePtr_[vertI_].pos = d3d_->PixelXMFLOAT3(at->right, at->top); // trc

   spritePtr_[vertI_].tex0 = trcTex;
   vertI_++;

   spritePtr_[vertI_].pos = d3d_->PixelXMFLOAT3(at->right, at->bottom); // brc

   spritePtr_[vertI_].tex0 = brcTex;
   vertI_++;

   spritePtr_[vertI_].pos = d3d_->PixelXMFLOAT3(at->left, at->top); // tlc

   spritePtr_[vertI_].tex0 = tlcTex;
   vertI_++;

   spritePtr_[vertI_].pos = d3d_->PixelXMFLOAT3(at->left, at->bottom); // blc

   spritePtr_[vertI_].tex0 = blcTex;
   vertI_++;

   End();
}
