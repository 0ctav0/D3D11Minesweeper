#include "pch.h"
#include "DeviceManager.h"
#include "Game.h"

namespace Texture {
   RECT CELL_RECT = { 0, 0, CELL_WIDTH, CELL_HEIGHT };
}

void Game::GetDefaultSize(long& width, long& height) {
   width = CELLS_X * CELL_WIDTH;
   height = CELLS_Y * CELL_HEIGHT;
}

bool Game::Init(HINSTANCE hInstance, HWND hwnd) {

   hInstance_ = hInstance;
   hwnd_ = hwnd;

   RECT dimensions;
   GetClientRect(hwnd, &dimensions);

   width_ = dimensions.right - dimensions.left;
   height_ = dimensions.bottom - dimensions.top;

   d3d_ = DeviceManager();
   d3d_.Init(hwnd, width_, height_);

   return LoadContent();
}

bool Game::LoadContent() {
   textureSpriteBatch_ = std::make_unique<DirectX::DX11::SpriteBatch>(d3d_.ctx_.Get());
   states_ = std::make_unique<DirectX::DX11::CommonStates>(d3d_.device_.Get());

   Microsoft::WRL::ComPtr<ID3D11Resource> resource;
   DX::ThrowIfFailed(
      DirectX::CreateWICTextureFromFile(d3d_.device_.Get(), TEXTURE_FILENAME, resource.GetAddressOf(),
         texture_.ReleaseAndGetAddressOf()));

   Microsoft::WRL::ComPtr<ID3D11Texture2D> cell;
   DX::ThrowIfFailed(resource.As(&cell));

   CD3D11_TEXTURE2D_DESC cellDesc;
   cell->GetDesc(&cellDesc);

   origin_.x = 0;
   origin_.y = 0;

   tileRect_.left = 0;
   tileRect_.right = cellDesc.Width * CELLS_X;
   tileRect_.top = 0;
   tileRect_.bottom = cellDesc.Height * CELLS_Y;

   screenPos_.x = 0;
   screenPos_.y = 0;

   return true;
}

void Game::Update(float dt) {}

void Game::Render() {
   if (d3d_.ctx_ == 0) return;

   d3d_.ctx_->ClearRenderTargetView(d3d_.renderTargetView_.Get(),
      DirectX::Colors::Gray);

   textureSpriteBatch_->Begin(DirectX::DX11::SpriteSortMode::SpriteSortMode_Deferred, states_->NonPremultiplied(), states_->LinearWrap());

   for (auto x = 0; x < CELLS_X; x++) {
      for (auto y = 0; y < CELLS_Y; y++) {
         DirectX::XMFLOAT2 at = { float(x * CELL_WIDTH), float(y * CELL_HEIGHT) };
         textureSpriteBatch_->Draw(texture_.Get(), at, &Texture::CELL_RECT, DirectX::Colors::White, 0.f, origin_);
      }
   }

   textureSpriteBatch_->End();

   d3d_.swapChain_->Present(1, 0);
}