#include "pch.h"
#include "DeviceManager.h"
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
   RECT QUESTION_MARK_RECT = { CELL_WIDTH * 4, 0, CELL_WIDTH * 5, CELL_HEIGHT };
   auto constexpr NUMBER_TOP_AT = 96;
   auto constexpr NUMBER_BOTTOM_AT = 158;

   auto constexpr SCALING = .5f;
};

auto constexpr CELL_WIDTH = Texture::CELL_WIDTH * Texture::SCALING;
auto constexpr CELL_HEIGHT = Texture::CELL_HEIGHT * Texture::SCALING;
auto constexpr NUMBER_WIDTH_HALF = CELL_WIDTH / 2 * Texture::SCALING;
auto constexpr NUMBER_HEIGHT_HALF = CELL_HEIGHT / 2 * Texture::SCALING;

Game::~Game() {
   Log::file.close();
   if (audioEngine_) {
      audioEngine_->Suspend();
   }
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
   if (gameState_ != GameState::Play) return;
   auto mouse = mouse_->GetState();
   selectedCell_.x = mouse.x / CELL_WIDTH;
   selectedCell_.y = mouse.y / CELL_HEIGHT;
}

bool Game::Init(HINSTANCE hInstance, HWND hwnd) {
   hInstance_ = hInstance;
   hwnd_ = hwnd;

   RECT dimensions;
   GetClientRect(hwnd, &dimensions);

   width_ = dimensions.right - dimensions.left;
   height_ = dimensions.bottom - dimensions.top;

   Log::file.open("log.txt");

   d3d_ = DeviceManager();
   auto d3dSuccess = d3d_.Init(hwnd, width_, height_);

   keyboard_ = std::make_unique<DirectX::Keyboard>();
   mouse_ = std::make_unique<DirectX::Mouse>();
   mouse_->SetWindow(hwnd);

   DirectX::AUDIO_ENGINE_FLAGS eflags = DirectX::AudioEngine_Default;
#ifdef _DEBUG
   eflags |= DirectX::AudioEngine_Debug;
#endif
   audioEngine_ = std::make_unique<DirectX::AudioEngine>(eflags);

   defeatSound_ = std::make_unique<DirectX::SoundEffect>(audioEngine_.get(), L"sounds/defeat.wav");
   winSound_ = std::make_unique<DirectX::SoundEffect>(audioEngine_.get(), L"sounds/win.wav");

   InitCells();

   return d3dSuccess && LoadContent();
}

void Game::InitCells() {
   for (auto x = 0; x < CELLS_X; x++) {
      for (auto y = 0; y < CELLS_Y; y++) {
         cells_[x][y] = Cell();
      }
   }
}

void Game::InitMines(int originX, int originY) {
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
      if (cell->mined || (originX == x && originY == y)) continue;
      cell->mined = true;
      i++;
   }
}

Cell* Game::GetCell(int x, int y) {
   return &cells_[x][y];
}

void Game::OpenAt(int x, int y) {
   auto cell = GetCell(x, y);
   if (cell->IsMarked() || cell->opened) return;

   cell->opened = true;
   if (!hasOpened_) InitMines(x, y);
   hasOpened_ = true;

   if (cell->mined) {
      Defeat();
      return;
   }
   auto mines = 0;
   IterateNear(x, y, [this, &mines](int x, int y) {
      auto cell = GetCell(x, y);
      mines += cell->mined ? 1 : 0;
      });
   cell->minesNear = mines;

   opened_++;
   DX::Print(L"cell[%i,%i]; total opened = %i\n", x, y, opened_);
   if (opened_ == NEED_TO_OPEN) Win();
}

void Game::IterateAll(std::function<void(Cell* cell)> cb) {
   for (auto begin = &cells_[0][0]; begin <= &cells_[CELLS_X - 1][CELLS_Y - 1]; begin++) {
      cb(begin);
   }
}

void Game::IterateNear(int originX, int originY, std::function<void(int, int)> cb) {
   auto minX = std::max(0, originX - 1);
   auto minY = std::max(0, originY - 1);
   auto maxX = std::min(CELLS_X - 1, originX + 1);
   auto maxY = std::min(CELLS_Y - 1, originY + 1);
   auto mines = 0;
   for (auto x = minX; x <= maxX; x++) {
      for (auto y = minY; y <= maxY; y++) {
         cb(x, y);
      }
   }
}

void Game::ExploreMap(int originX, int originY) {
   auto cell = GetCell(originX, originY);
   if (cell->opened || cell->IsMarked()) return;
   OpenAt(originX, originY);
   if (cell->minesNear == 0) {
      IterateNear(originX, originY, [this](int x, int y) {
         ExploreMap(x, y);
         });
   }
}

void Game::OpenNearForced(int originX, int originY) {
   auto cell = GetCell(originX, originY);
   if (!(cell->opened && cell->minesNear > 0)) return;
   auto flagged = 0;
   IterateNear(originX, originY, [this, &flagged](int x, int y) {
      auto cell = GetCell(x, y);
      flagged += cell->state == RCellState::Flagged ? 1 : 0;
      });
   if (cell->minesNear == flagged) {
      IterateNear(originX, originY, [this](int x, int y) {
         ExploreMap(x, y);
         });
   }
}

void Game::PressedAround(int originX, int originY) {
   auto cell = GetCell(originX, originY);
   cell->pressed = cell->IsMarked() ? false : true;
   if (cell->minesNear > 0) {
      IterateNear(originX, originY, [this](int x, int y) {
         auto cell = GetCell(x, y);
         cell->pressed = cell->IsMarked() ? false : true;
         });
   }
}

void Game::UnpressedAll() {
   IterateAll([](Cell* cell) {
      cell->pressed = false;
      });
}

void Game::ClickAt(int x, int y) {
   auto cell = GetCell(x, y);
   if (cell->opened && cell->minesNear > 0) {
      return OpenNearForced(x, y);
   }
   ExploreMap(x, y);
}

void Game::MarkAt(int x, int y) {
   auto cell = GetCell(x, y);
   if (!cell->opened) cell->ToggleState();
}

bool Game::IsCellSelected(int x, int y) {
   auto cell = GetCell(x, y);
   return selectedCell_.x == x && selectedCell_.y == y && !cell->IsMarked();
}

void Game::Defeat() {
   gameState_ = GameState::Defeat;
   defeatSound_->Play();
}

void Game::Win() {
   gameState_ = GameState::Win;
   winSound_->Play();
}

bool Game::LoadContent() {
   Log::Info("Game::LoadContent start");

   textureSpriteBatch_ = std::make_unique<DirectX::DX11::SpriteBatch>(d3d_.ctx_.Get());
   states_ = std::make_unique<DirectX::DX11::CommonStates>(d3d_.device_.Get());

   Microsoft::WRL::ComPtr<ID3D11Resource> resource;
   DX::ThrowIfFailed(DirectX::CreateWICTextureFromFile(d3d_.device_.Get(),
      Texture::FILENAME, resource.GetAddressOf(), texture_.ReleaseAndGetAddressOf()), "Failed to create a texture from a file");

   Microsoft::WRL::ComPtr<ID3D11Texture2D> cell;
   DX::ThrowIfFailed(resource.As(&cell), "Failed to set a resource");

   CD3D11_TEXTURE2D_DESC cellDesc;
   cell->GetDesc(&cellDesc);

   origin_.x = 0;
   origin_.y = 0;

   tileRect_.left = 0;
   tileRect_.right = cellDesc.Width * CELLS_X;
   tileRect_.top = 0;
   tileRect_.bottom = cellDesc.Height * CELLS_Y;

   Log::Info("Game::LoadContent end");

   return true;
}

void Game::Update(float dt) {
   auto kb = keyboard_->GetState();
   auto mouseState = mouse_->GetState();
   keyTracker_.Update(kb);
   mouseTracker_.Update(mouseState);

   if (!audioEngine_->Update()) {

   }

   if (keyTracker_.IsKeyReleased(DirectX::Keyboard::Escape)) {
      ExitGame();
   }

   if (gameState_ != GameState::Play) return;

   leftHeld_ = mouseTracker_.leftButton == DirectX::Mouse::ButtonStateTracker::HELD;

   UnpressedAll();


   if (leftHeld_) {
      PressedAround(selectedCell_.x, selectedCell_.y);
   }

   if (mouseTracker_.leftButton == DirectX::Mouse::ButtonStateTracker::RELEASED) {
      ClickAt(selectedCell_.x, selectedCell_.y);
   }

   if (mouseTracker_.rightButton == DirectX::Mouse::ButtonStateTracker::RELEASED) {
      MarkAt(selectedCell_.x, selectedCell_.y);
   }
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
         DirectX::XMFLOAT2 at = { float(x * CELL_WIDTH), float(y * CELL_HEIGHT) };
         auto cell = GetCell(x, y);
         if (!cell->opened) {
            if (cell->pressed) {
               continue;
            }
            auto rect = &Texture::CELL_RECT;
            textureSpriteBatch_->Draw(texture_.Get(), at, rect,
               DirectX::Colors::White, 0.f, origin_, Texture::SCALING);
            if (cell->IsMarked()) {
               DirectX::XMFLOAT2 at = { float(x * CELL_WIDTH) + 6,
                                       float(y * CELL_HEIGHT) + 2 };
               auto texture = cell->state == RCellState::Flagged ? &Texture::FLAG_RECT : &Texture::QUESTION_MARK_RECT;
               textureSpriteBatch_->Draw(texture_.Get(), at, texture,
                  DirectX::Colors::White, 0.f, origin_, Texture::SCALING);
            }
         }
         else if (cell->mined) {
            textureSpriteBatch_->Draw(texture_.Get(), at, &Texture::MINE_RECT,
               DirectX::Colors::White, 0.f, origin_, Texture::SCALING);
         }
         else if (cell->minesNear > 0) {

            DirectX::XMFLOAT2 at = { float(x * CELL_WIDTH) + NUMBER_WIDTH_HALF,
                                    float(y * CELL_HEIGHT) + NUMBER_HEIGHT_HALF };
            auto left = (cell->minesNear - 1) * Texture::NUMBER_WIDTH;
            auto right = cell->minesNear * Texture::NUMBER_WIDTH;
            RECT rc = { left, Texture::NUMBER_TOP_AT, right, Texture::NUMBER_BOTTOM_AT };
            textureSpriteBatch_->Draw(texture_.Get(), at, &rc, DirectX::Colors::White, 0.f, origin_, Texture::SCALING * Texture::SCALING);
         }
      }
   }

   textureSpriteBatch_->End();

   d3d_.swapChain_->Present(1, 0);
}