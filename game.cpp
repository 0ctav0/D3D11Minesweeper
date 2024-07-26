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
   RECT MINUS = { 56, 152, 88, 156 };

   auto constexpr NUMBER_TOP_AT = 96;
   auto constexpr NUMBER_BOTTOM_AT = 158;

   auto constexpr SCALING = .5f;

   RECT GetDigitRect(BYTE digit) {
      auto left = digit * Texture::NUMBER_WIDTH;
      auto right = (digit + 1) * Texture::NUMBER_WIDTH;
      RECT rc = { left, Texture::NUMBER_TOP_AT, right, Texture::NUMBER_BOTTOM_AT };
      return rc;
   }
};

auto constexpr CELL_WIDTH = Texture::CELL_WIDTH * Texture::SCALING;
auto constexpr CELL_HEIGHT = Texture::CELL_HEIGHT * Texture::SCALING;
auto constexpr NUMBER_WIDTH_HALF = CELL_WIDTH / 2 * Texture::SCALING;
auto constexpr NUMBER_HEIGHT_HALF = CELL_HEIGHT / 2 * Texture::SCALING;

namespace UI {
   auto constexpr MINES_COUNT_CHAR_NUMBER = 3;
   auto constexpr TOP_PANEL_WIDTH = CELLS_X * CELL_WIDTH;
   auto constexpr TOP_PANEL_HEIGHT = Texture::CELL_HEIGHT * 2;
   RECT TOP_LEFT_CORNER = { 0, 0, 5, 5 };
   RECT TOP_RIGHT_CORNER = { 60, 0, 64, 5 };
   RECT BOTTOM_LEFT_CORNER = { 0, 60, 5, 64 };
   RECT BOTTOM_RIGHT_CORNER = { 60, 60, 64, 64 };
   RECT TOP_HORIZONTAL_LINE = { 5, 0, 6, 5 };
   RECT BOTTOM_HORIZONTAL_LINE = { 5, 60, 6, 64 };
   RECT LEFT_VERTICAL_LINE = { 0, 5, 5, 6 };
   RECT RIGHT_VERTICAL_LINE = { 60, 5, 64, 6 };
   RECT BACKGROUND_RECT = { 5,5, 6,6 };
}

Game::~Game() {
   Log::file.close();
   if (audioEngine_) {
      audioEngine_->Suspend();
   }
}

void Game::GetDefaultSize(long& width, long& height) {
   width = CELLS_X * CELL_WIDTH;
   height = CELLS_Y * CELL_HEIGHT + UI::TOP_PANEL_HEIGHT;
}

bool Game::ExitGame() {
   PostQuitMessage(0);
   return true;
}

void Game::OnMouseMove() {
   if (gameState_ != GameState::Play) return;
   auto mouse = mouse_->GetState();
   int x = mouse.x / CELL_WIDTH;
   int y = std::floor((mouse.y - UI::TOP_PANEL_HEIGHT) / CELL_HEIGHT);
   DX::Print(L"[%i,%i]", x, y);
   selectedCell_.x = x;
   selectedCell_.y = y;
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
   if (gameState_ != GameState::Play || cell->opened || cell->IsMarked()) return;
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
   if (!cell->opened) {
      cell->ToggleState();
      if (cell->state == RCellState::Flagged) flagged_++;
      if (cell->state == RCellState::Questioned) flagged_--;
   }
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

std::vector<char> Game::GetDigits(int number) {
   std::vector<char> digits = {};
   int rest = number >= std::pow(10, UI::MINES_COUNT_CHAR_NUMBER) ?
      std::pow(10, UI::MINES_COUNT_CHAR_NUMBER) - 1 :
      number <= -std::pow(10, UI::MINES_COUNT_CHAR_NUMBER - 1) ?
      std::pow(10, UI::MINES_COUNT_CHAR_NUMBER - 1) - 1 :
      number;
   auto i = 0;
   do {
      auto digit = std::abs(rest % 10);
      digits.push_back(digit);
      rest /= 10;
      i++;
   } while (rest != 0);
   if (number < 0) digits.push_back('-');
   std::reverse(digits.begin(), digits.end());
   return digits;
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
      if (selectedCell_.IsInBounds()) PressedAround(selectedCell_.x, selectedCell_.y);
   }

   if (mouseTracker_.leftButton == DirectX::Mouse::ButtonStateTracker::RELEASED) {
      if (selectedCell_.IsInBounds()) ClickAt(selectedCell_.x, selectedCell_.y);
   }

   if (mouseTracker_.rightButton == DirectX::Mouse::ButtonStateTracker::RELEASED) {
      if (selectedCell_.IsInBounds()) MarkAt(selectedCell_.x, selectedCell_.y);
   }
}

void Game::Draw(DirectX::XMFLOAT2 const& pos, RECT const* sourceRectangle, DirectX::FXMVECTOR color = DirectX::Colors::White, float scaling = 1) {
   textureSpriteBatch_->Draw(texture_.Get(), pos, sourceRectangle, color, .0f, origin_, scaling);
}

void Game::RenderPanel(RECT size) {
   auto width = size.right - size.left;
   auto height = size.bottom - size.top;

   // top left corner
   DirectX::XMFLOAT2 tlcAt = { float(size.left), float(size.top) };
   auto tlcWidth = UI::TOP_LEFT_CORNER.right - UI::TOP_LEFT_CORNER.left;
   auto tlcHeight = UI::TOP_LEFT_CORNER.bottom - UI::TOP_LEFT_CORNER.top;
   Draw(tlcAt, &UI::TOP_LEFT_CORNER);

   // top right corner
   auto trcWidth = UI::TOP_RIGHT_CORNER.right - UI::TOP_RIGHT_CORNER.left;
   DirectX::XMFLOAT2 trcAt = { size.left + float(width - trcWidth), float(size.top) };
   Draw(trcAt, &UI::TOP_RIGHT_CORNER);

   // bottom left corner
   auto blcHeight = UI::BOTTOM_LEFT_CORNER.bottom - UI::BOTTOM_LEFT_CORNER.top;
   DirectX::XMFLOAT2 blcAt = { float(size.left), size.top + float(height - blcHeight) };
   Draw(blcAt, &UI::BOTTOM_LEFT_CORNER);

   // bottom right corner
   auto brcWidth = UI::BOTTOM_RIGHT_CORNER.right - UI::BOTTOM_RIGHT_CORNER.left;
   auto brcHeight = UI::BOTTOM_RIGHT_CORNER.bottom - UI::BOTTOM_RIGHT_CORNER.top;
   DirectX::XMFLOAT2 brcAt = { size.left + float(width - brcWidth), size.top + float(height - brcHeight) };
   Draw(brcAt, &UI::BOTTOM_RIGHT_CORNER);

   for (auto x = tlcWidth; x <= width - trcWidth; x++) { // horizontally
      DirectX::XMFLOAT2 at = { size.left + float(x), float(size.top) };
      Draw(at, &UI::TOP_HORIZONTAL_LINE);
      at.y = size.top + height - blcHeight;
      Draw(at, &UI::BOTTOM_HORIZONTAL_LINE);
   }

   for (auto y = tlcHeight; y <= height - blcHeight; y++) { // vertically
      DirectX::XMFLOAT2 at = { float(size.left), size.top + float(y) };
      Draw(at, &UI::LEFT_VERTICAL_LINE);
      at.x = size.left + width - trcWidth;
      Draw(at, &UI::RIGHT_VERTICAL_LINE);
   }

   for (auto x = tlcWidth; x <= width - trcWidth; x++) { // fill center
      for (auto y = tlcHeight; y <= height - blcHeight; y++) {
         DirectX::XMFLOAT2 at = { size.left + float(x), size.top + float(y) };
         Draw(at, &UI::BACKGROUND_RECT);
      }
   }
}

void Game::RenderTopPanel() {
   long width, height;
   GetDefaultSize(width, height);

   textureSpriteBatch_->Begin(
      DirectX::DX11::SpriteSortMode::SpriteSortMode_Deferred,
      states_->NonPremultiplied(), states_->LinearWrap());

   RECT size = { 0, 0, width, UI::TOP_PANEL_HEIGHT };
   RenderPanel(size);

   RenderMinesNumber();

   textureSpriteBatch_->End();
}

void Game::RenderMinesNumber() {
   DirectX::XMFLOAT2 at = { float(UI::TOP_LEFT_CORNER.right + 5), float(UI::TOP_LEFT_CORNER.bottom + 20) };
   int minesAndFlagged = MINES_COUNT - flagged_;
   auto digits = GetDigits(minesAndFlagged);

   RECT size = { at.x, at.y - 5, at.x + Texture::NUMBER_WIDTH * UI::MINES_COUNT_CHAR_NUMBER + 4, at.y + Texture::NUMBER_HEIGHT + 4 };
   RenderPanel(size);
   // indent
   auto indentNumber = UI::MINES_COUNT_CHAR_NUMBER - digits.size();
   for (auto i = 0; i < indentNumber; i++) {
      at.x += Texture::NUMBER_WIDTH;
   }
   // digits
   for (auto digit : digits) {
      if (digit == '-') {
         DirectX::XMFLOAT2 minusAt = { at.x, at.y + Texture::NUMBER_HEIGHT / 2 };
         Draw(minusAt, &Texture::MINUS, DirectX::Colors::DarkRed);
      }
      else {
         auto rect = Texture::GetDigitRect(digit);
         Draw(at, &rect, DirectX::Colors::DarkRed);
      }
      at.x += Texture::NUMBER_WIDTH;
   }

}

void Game::RenderGameField() {
   textureSpriteBatch_->Begin(
      DirectX::DX11::SpriteSortMode::SpriteSortMode_Deferred,
      states_->NonPremultiplied(), states_->LinearWrap());

   for (auto x = 0; x < CELLS_X; x++) {
      for (auto y = 0; y < CELLS_Y; y++) {
         DirectX::XMFLOAT2 at = { float(x * CELL_WIDTH), float(y * CELL_HEIGHT) + UI::TOP_PANEL_HEIGHT };
         auto cell = GetCell(x, y);
         if (!cell->opened) {
            auto color = cell->pressed ? DirectX::Colors::Red : DirectX::Colors::White;
            Draw(at, &Texture::CELL_RECT, color, Texture::SCALING);
            if (gameState_ == GameState::Defeat && cell->mined) {
               Draw(at, &Texture::MINE_RECT, DirectX::Colors::White, Texture::SCALING);
            }
            if (cell->IsMarked()) {
               DirectX::XMFLOAT2 at = { float(x * CELL_WIDTH) + 6,
                                       float(y * CELL_HEIGHT) + 2 + UI::TOP_PANEL_HEIGHT };
               auto texture = cell->state == RCellState::Flagged ? &Texture::FLAG_RECT : &Texture::QUESTION_MARK_RECT;
               Draw(at, texture, DirectX::Colors::White, Texture::SCALING);
            }
         }
         else if (cell->mined) {
            Draw(at, &Texture::MINE_RECT, DirectX::Colors::White, Texture::SCALING);
         }
         else if (cell->minesNear > 0) {
            DirectX::XMFLOAT2 at = { float(x * CELL_WIDTH) + NUMBER_WIDTH_HALF,
                                    float(y * CELL_HEIGHT) + NUMBER_HEIGHT_HALF + UI::TOP_PANEL_HEIGHT };
            auto rect = Texture::GetDigitRect(cell->minesNear);
            Draw(at, &rect, NUMBER_TINTS[cell->minesNear - 1], Texture::SCALING * Texture::SCALING);
         }
      }
   }

   textureSpriteBatch_->End();
}

void Game::Render() {
   if (d3d_.ctx_ == 0) return;

   d3d_.ctx_->ClearRenderTargetView(d3d_.renderTargetView_.Get(),
      DirectX::Colors::Gray);

   RenderTopPanel();
   RenderGameField();

   d3d_.swapChain_->Present(1, 0);
}