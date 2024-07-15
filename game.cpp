#include "pch.h"
#include "DeviceManager.h"
#include "Controller.h"
#include "Game.h"

namespace Texture {
   RECT CELL_RECT = { 0, 0, CELL_WIDTH, CELL_HEIGHT };
   RECT SELECTED_CELL_RECT = { CELL_WIDTH, 0, CELL_WIDTH * 2, CELL_HEIGHT };
   RECT FLAG_RECT = { CELL_WIDTH * 2, 0, CELL_WIDTH * 3, CELL_HEIGHT };
}

void Game::GetDefaultSize(long& width, long& height) {
   width = CELLS_X * CELL_WIDTH;
   height = CELLS_Y * CELL_HEIGHT;
}

bool Game::ExitGame() {
   PostQuitMessage(0);
   return true;
}

void Game::OnMouseMove() {
   auto mouse = cntrl_.mouse_->GetState();
   selectedCell_.x = mouse.x / CELL_WIDTH;
   selectedCell_.y = mouse.y / CELL_HEIGHT;
}

void Game::OnMouseLDown() {
   Cell* cell = &cells_[std::format("{},{}", selectedCell_.x, selectedCell_.y)];
   if (!cell->flagged)
      cell->opened = true;
}

void Game::OnMouseRDown() {
   Cell* cell = &cells_[std::format("{},{}", selectedCell_.x, selectedCell_.y)];
   if (!cell->opened)
      cell->flagged = !cell->flagged;
}

bool Game::Init(HINSTANCE hInstance, HWND hwnd) {

   hInstance_ = hInstance;
   hwnd_ = hwnd;

   RECT dimensions;
   GetClientRect(hwnd, &dimensions);

   width_ = dimensions.right - dimensions.left;
   height_ = dimensions.bottom - dimensions.top;

   d3d_ = DeviceManager();
   auto d3dSuccess = d3d_.Init(hwnd, width_, height_);
   cntrl_ = Controller();
   auto cntrlSuccess = cntrl_.Init(hwnd);

   for (auto x = 0; x < CELLS_X; x++) {
      for (auto y = 0; y < CELLS_Y; y++) {
         cells_.insert(std::pair{ std::format("{},{}", x, y), Cell() });
      }
   }

   return d3dSuccess && cntrlSuccess && LoadContent();
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

void Game::Update(float dt) {
   auto kb = cntrl_.keyboard_->GetState();
   auto mouse = cntrl_.mouse_->GetState();
   mouseX_ = mouse.x; mouseY_ = mouse.y;

   if (kb.Escape) {
      ExitGame();
   }

}

void Game::Render() {
   if (d3d_.ctx_ == 0) return;

   d3d_.ctx_->ClearRenderTargetView(d3d_.renderTargetView_.Get(),
      DirectX::Colors::Gray);

   textureSpriteBatch_->Begin(DirectX::DX11::SpriteSortMode::SpriteSortMode_Deferred, states_->NonPremultiplied(), states_->LinearWrap());

   for (auto x = 0; x < CELLS_X; x++) {
      for (auto y = 0; y < CELLS_Y; y++) {
         DirectX::XMFLOAT2 at = { float(x * CELL_WIDTH), float(y * CELL_HEIGHT) };
         Cell* cell = &cells_[std::format("{},{}", x, y)];
         if (!cell->opened) {
            RECT* rect = selectedCell_.x == x && selectedCell_.y == y && !cell->flagged ? &Texture::SELECTED_CELL_RECT : &Texture::CELL_RECT;
            textureSpriteBatch_->Draw(texture_.Get(), at, rect, DirectX::Colors::White, 0.f, origin_);
            if (cell->flagged) {
               DirectX::XMFLOAT2 at = { float(x * CELL_WIDTH) + 6, float(y * CELL_HEIGHT) + 2 };
               textureSpriteBatch_->Draw(texture_.Get(), at, &Texture::FLAG_RECT, DirectX::Colors::White, 0.f, origin_);
            }
         }
      }
   }

   textureSpriteBatch_->End();

   d3d_.swapChain_->Present(1, 0);
}