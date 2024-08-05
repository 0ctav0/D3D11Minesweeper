#include "pch.h"
#include "Types.h"
#include "DeviceManager.h"
#include "SpriteRenderer.h"

bool SpriteRenderer::Init(DeviceManager* d3d, const wchar_t* filename) {
   d3d_ = d3d;
   Microsoft::WRL::ComPtr<ID3D11Resource> resource;
   DX::ThrowIfFailed(DirectX::CreateWICTextureFromFile(d3d_->device_.Get(),
      filename, resource.GetAddressOf(), texture_.ReleaseAndGetAddressOf()), "Failed to create a texture from a file");

   Microsoft::WRL::ComPtr<ID3D11Texture2D> tex;
   DX::ThrowIfFailed(resource.As(&tex), "Failed resource as tex");

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

void SpriteRenderer::Draw(const RECT* at, const RECT* tex, const DirectX::XMVECTORF32* color) {
   spritePtr_[vertI_].pos = d3d_->PixelXMFLOAT3(at->right, at->top); // trc
   DirectX::XMStoreFloat4(&spritePtr_[vertI_].color, *color);
   spritePtr_[vertI_].tex0 = XMFLOAT2(tex->right / width_, tex->top / height_);
   vertI_++;

   spritePtr_[vertI_].pos = d3d_->PixelXMFLOAT3(at->right, at->bottom); // brc
   DirectX::XMStoreFloat4(&spritePtr_[vertI_].color, *color);
   spritePtr_[vertI_].tex0 = XMFLOAT2(tex->right / width_, tex->bottom / height_);
   vertI_++;

   spritePtr_[vertI_].pos = d3d_->PixelXMFLOAT3(at->left, at->top); // tlc
   DirectX::XMStoreFloat4(&spritePtr_[vertI_].color, *color);
   spritePtr_[vertI_].tex0 = XMFLOAT2(tex->left / width_, tex->top / height_);
   vertI_++;

   spritePtr_[vertI_].pos = d3d_->PixelXMFLOAT3(at->left, at->bottom); // blc
   DirectX::XMStoreFloat4(&spritePtr_[vertI_].color, *color);
   spritePtr_[vertI_].tex0 = XMFLOAT2(tex->left / width_, tex->bottom / height_);
   vertI_++;
}

void SpriteRenderer::Draw(const RECT* at, const RECT* tex) {
   Draw(at, tex, &DirectX::Colors::White);
}
