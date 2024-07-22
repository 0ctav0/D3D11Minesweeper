﻿#include "pch.h"
#include "DeviceManager.h"
#include "Controller.h"
#include "Game.h"

namespace Texture {
   auto constexpr FILENAME = L"img/texture.png";
   auto constexpr CELL_WIDTH = 64;
   auto constexpr CELL_HEIGHT = 64;
   auto constexpr NUMBER_WIDTH = 48;
   auto constexpr NUMBER_HEIGHT = 63;
   RECT CELL_RECT = { 0, 0, CELL_WIDTH, CELL_HEIGHT };
   RECT SELECTED_CELL_RECT = { CELL_WIDTH, 0, CELL_WIDTH * 2, CELL_HEIGHT };
   RECT FLAG_RECT = { CELL_WIDTH * 2, 0, CELL_WIDTH * 3, CELL_HEIGHT };
   RECT MINE_RECT = { CELL_WIDTH * 3, 0, CELL_WIDTH * 4, CELL_HEIGHT };
   auto constexpr NUMBER_TOP_AT = 96;
   auto constexpr NUMBER_BOTTOM_AT = 158;
}

void Game::GetDefaultSize(long& width, long& height) {
   width = CELLS_X * Texture::CELL_WIDTH;
   height = CELLS_Y * Texture::CELL_HEIGHT;
}

bool Game::ExitGame() {
   PostQuitMessage(0);
   return true;
}

void Game::OnMouseMove() {
   auto* mouse = cntrl_.GetMouseState();
   selectedCell_.x = mouse->x / Texture::CELL_WIDTH;
   selectedCell_.y = mouse->y / Texture::CELL_HEIGHT;
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

   InitMines();

   return d3dSuccess && cntrlSuccess && LoadContent();
}

void Game::InitMines() {
   for (auto x = 0; x < CELLS_X; x++) {
      for (auto y = 0; y < CELLS_Y; y++) {
         cells_.insert(std::pair{ std::format("{},{}", x, y), Cell() });
      }
   }

   std::random_device dev;
   std::mt19937 rng(dev());
   std::uniform_int_distribution<std::mt19937::result_type> distX(0,
      CELLS_X - 1);
   std::uniform_int_distribution<std::mt19937::result_type> distY(0,
      CELLS_Y - 1);

   auto i = 0;
   while (i < MINES_COUNT) {
      auto x = distX(rng);
      auto y = distY(rng);
      auto cell = GetCell(x, y);
      if (!cell->mined) {
         cell->mined = true;
         i++;
      }
   }
}

Cell* Game::GetCell(int x, int y) {
   return &cells_[std::format("{},{}", x, y)];
}

void Game::OpenAt(int x, int y) {
   auto cell = GetCell(x, y);
   if (!cell->flagged) cell->opened = true;
}

void Game::IterateNear(int originX, int originY, std::function<void(int, int)> cb) {
   auto minX = std::max(0, originX - 1);
   auto minY = std::max(0, originY - 1);
   auto maxX = std::min(CELLS_X, originX + 1);
   auto maxY = std::min(CELLS_Y, originY + 1);
   auto mines = 0;
   for (auto x = minX; x <= maxX; x++) {
      for (auto y = minY; y <= maxY; y++) {
         cb(x, y);
      }
   }
}

void Game::ExploreMap(int originX, int originY) {
   auto cell = GetCell(originX, originY);
   if (cell->opened) return;
   OpenAt(originX, originY);
   //if (cell->mined) return;
   auto mines = 0;
   IterateNear(originX, originY, [this, &mines](int x, int y) {
      auto cell = GetCell(x, y);
      mines += cell->mined ? 1 : 0;
      });
   cell->minesNear = mines;
   if (mines == 0) {
      IterateNear(originX, originY, [this, &mines](int x, int y) {
         ExploreMap(x, y);
         });
   }
}

void Game::FlagAt(int x, int y) {
   auto cell = GetCell(x, y);
   if (!cell->opened) cell->flagged = !cell->flagged;
}

bool Game::IsCellSelected(int x, int y) {
   auto cell = GetCell(x, y);
   return selectedCell_.x == x && selectedCell_.y == y && !cell->flagged;
}

bool Game::LoadContent() {
   textureSpriteBatch_ = std::make_unique<DirectX::DX11::SpriteBatch>(d3d_.ctx_.Get());
   states_ = std::make_unique<DirectX::DX11::CommonStates>(d3d_.device_.Get());

   Microsoft::WRL::ComPtr<ID3D11Resource> resource;
   DX::ThrowIfFailed(DirectX::CreateWICTextureFromFile(d3d_.device_.Get(),
      Texture::FILENAME, resource.GetAddressOf(), texture_.ReleaseAndGetAddressOf()));

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

   return true;
}

void Game::Update(float dt) {
   cntrl_.BeforeUpdate();
   auto* kb = cntrl_.GetKeyboardState();

   if (kb->Escape) {
      ExitGame();
   }

   if (cntrl_.MouseReleased(&DirectX::Mouse::State::leftButton)) {
      ExploreMap(selectedCell_.x, selectedCell_.y);
   }

   if (cntrl_.MouseReleased(&DirectX::Mouse::State::rightButton)) {
      FlagAt(selectedCell_.x, selectedCell_.y);
   }

   cntrl_.AfterUpdate();
}

void Game::Render() {
   if (d3d_.ctx_ == 0) return;

   d3d_.ctx_->ClearRenderTargetView(d3d_.renderTargetView_.Get(),
      DirectX::Colors::Gray);

   textureSpriteBatch_->Begin(
      DirectX::DX11::SpriteSortMode::SpriteSortMode_Deferred,
      states_->NonPremultiplied(), states_->LinearWrap());

   for (auto x = 0; x < CELLS_X; x++) {
      for (auto y = 0; y < CELLS_Y; y++) {
         DirectX::XMFLOAT2 at = { float(x * Texture::CELL_WIDTH), float(y * Texture::CELL_HEIGHT) };
         auto cell = GetCell(x, y);
         if (!cell->opened) {
            RECT* rect = IsCellSelected(x, y) ? &Texture::SELECTED_CELL_RECT
               : &Texture::CELL_RECT;
            textureSpriteBatch_->Draw(texture_.Get(), at, rect,
               DirectX::Colors::White, 0.f, origin_);
            if (cell->flagged) {
               DirectX::XMFLOAT2 at = { float(x * Texture::CELL_WIDTH) + 6,
                                       float(y * Texture::CELL_HEIGHT) + 2 };
               textureSpriteBatch_->Draw(texture_.Get(), at, &Texture::FLAG_RECT,
                  DirectX::Colors::White, 0.f, origin_);
            }
         }
         else if (cell->mined) {
            textureSpriteBatch_->Draw(texture_.Get(), at, &Texture::MINE_RECT,
               DirectX::Colors::White, 0.f, origin_);
         }
         else if (cell->minesNear > 0) {
            auto constexpr scaling = 0.5f;
            auto constexpr numberWidth = Texture::NUMBER_WIDTH * scaling;
            auto constexpr numberHeight = Texture::NUMBER_HEIGHT * scaling;
            auto constexpr numberWidthHalf = numberWidth / 2;
            auto constexpr numberHeightHalf = numberHeight / 2;
            DirectX::XMFLOAT2 at = { float(x * Texture::CELL_WIDTH) + numberWidthHalf,
                                    float(y * Texture::CELL_HEIGHT) + numberHeightHalf };
            auto left = (cell->minesNear - 1) * Texture::NUMBER_WIDTH;
            auto right = cell->minesNear * Texture::NUMBER_WIDTH;
            RECT rc = { left, Texture::NUMBER_TOP_AT, right, Texture::NUMBER_BOTTOM_AT };
            textureSpriteBatch_->Draw(texture_.Get(), at, &rc, DirectX::Colors::White, 0.f, origin_, scaling);
         }
      }
   }

   textureSpriteBatch_->End();

   d3d_.swapChain_->Present(1, 0);
}