#include "pch.h"
#include "Game.h"
#include "Types.h"
#include "DeviceManager.h"
#include "SoundSystem.h"

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
   RECT TOP_RIGHT_CORNER = { 59, 0, 64, 5 };
   RECT BOTTOM_LEFT_CORNER = { 0, 59, 5, 64 };
   RECT BOTTOM_RIGHT_CORNER = { 59, 59, 64, 64 };
   RECT TOP_HORIZONTAL_LINE = { 5, 0, 6, 5 };
   RECT BOTTOM_HORIZONTAL_LINE = { 5, 59, 6, 64 };
   RECT LEFT_VERTICAL_LINE = { 0, 5, 5, 6 };
   RECT RIGHT_VERTICAL_LINE = { 59, 5, 64, 6 };
   RECT BACKGROUND_RECT = { 5,5, 6,6 };
}

Game::~Game() {
   Log::file.close();
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
   if (data_.gameState != GameState::Play) return;
   auto mouse = mouse_->GetState();
   int x = mouse.x / CELL_WIDTH;
   int y = std::floor((mouse.y - UI::TOP_PANEL_HEIGHT) / CELL_HEIGHT);
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

   auto d3dSuccess = d3d_.Init(hwnd, width_, height_);
   auto soundSuccess = sound_.Init();
   auto spriteSuccess = sprite_.Init(&d3d_, Texture::FILENAME);

   keyboard_ = std::make_unique<DirectX::Keyboard>();
   mouse_ = std::make_unique<DirectX::Mouse>();
   mouse_->SetWindow(hwnd);

   InitCells();

   return d3dSuccess && soundSuccess && spriteSuccess && LoadContent();
}

void Game::InitCells() {
   for (auto x = 0; x < CELLS_X; x++) {
      for (auto y = 0; y < CELLS_Y; y++) {
         data_.cells[x][y] = Cell();
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
   return &data_.cells[x][y];
}

void Game::OpenAt(int x, int y) {
   auto cell = GetCell(x, y);
   if (cell->IsMarked() || cell->opened) return;

   cell->opened = true;

   if (!data_.started) {
      Start(x, y);
   }

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

   data_.opened++;
   if (data_.opened == NEED_TO_OPEN) Win();
}

void Game::IterateAll(std::function<void(Cell* cell)> cb) {
   for (auto begin = &data_.cells[0][0]; begin <= &data_.cells[CELLS_X - 1][CELLS_Y - 1]; begin++) {
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
   if (data_.gameState != GameState::Play || cell->opened || cell->IsMarked()) return;
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
      if (cell->state == RCellState::Flagged) data_.flagged++;
      if (cell->state == RCellState::Questioned) data_.flagged--;
   }
}

bool Game::IsCellSelected(int x, int y) {
   auto cell = GetCell(x, y);
   return selectedCell_.x == x && selectedCell_.y == y && !cell->IsMarked();
}

void Game::Start(int x, int y) {
   data_.started = true;

   InitMines(x, y);
}

void Game::Defeat() {
   data_.gameState = GameState::Defeat;
   sound_.defeat->Play();
}

void Game::Win() {
   data_.gameState = GameState::Win;
   sound_.win->Play();
}

void Game::Restart() {
   sound_.PlayPig();
   data_ = GameData();
}

std::vector<char> Game::GetDigits(int number) {
   std::vector<char> digits;
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


   Microsoft::WRL::ComPtr<ID3DBlob> vsBlob;
   Microsoft::WRL::ComPtr<ID3DBlob> psBlob;

   DX::ThrowIfFailed(D3DCompileFromFile(L"shader.fx", nullptr, nullptr, "VS_Main", "vs_4_0", 0, 0, vsBlob.GetAddressOf(), nullptr), "Failed to compile a shader from a file");

   DX::ThrowIfFailed(d3d_.device_->CreateVertexShader(vsBlob.Get()->GetBufferPointer(), vsBlob.Get()->GetBufferSize(), nullptr, vertexShader_.GetAddressOf()), "Failed to create vertex shader");

   DX::ThrowIfFailed(D3DCompileFromFile(L"shader.fx", nullptr, nullptr, "PS_Main", "ps_4_0", 0, 0, psBlob.GetAddressOf(), nullptr), "Failed to compile a shader from a file");

   DX::ThrowIfFailed(d3d_.device_->CreatePixelShader(psBlob.Get()->GetBufferPointer(), psBlob.Get()->GetBufferSize(), nullptr, pixelShader_.GetAddressOf()), "Failed to create pixel shader");


   D3D11_INPUT_ELEMENT_DESC inputDesc[] = {
      {"SV_POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
         D3D11_INPUT_PER_VERTEX_DATA, 0},
      {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
      {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12 + 16,
         D3D11_INPUT_PER_VERTEX_DATA, 0}
   };

   auto totalLayoutElements = ARRAYSIZE(inputDesc);

   DX::ThrowIfFailed(d3d_.device_->CreateInputLayout(
      inputDesc, totalLayoutElements, vsBlob->GetBufferPointer(),
      vsBlob->GetBufferSize(), inputLayout_.GetAddressOf()), "Failed to create an input layout");


   D3D11_SAMPLER_DESC samplerDesc = {};
   samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
   samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
   samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
   samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
   samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
   samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

   DX::ThrowIfFailed(d3d_.device_->CreateSamplerState(&samplerDesc,
      samplerState_.GetAddressOf()), "Failed to create sampler state");

   D3D11_BLEND_DESC blendDesc = {};
   blendDesc.RenderTarget[0].BlendEnable = TRUE;
   blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
   blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
   blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
   blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_INV_DEST_ALPHA;
   blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
   blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
   blendDesc.RenderTarget[0].RenderTargetWriteMask = 0x0F;

   float blendFactor[] = { .0f, .0f, .0f, .0f };

   d3d_.device_->CreateBlendState(&blendDesc, blendState_.GetAddressOf());
   d3d_.ctx_->OMSetBlendState(blendState_.Get(), blendFactor, 0xFFFFFFFF);



   Log::Info("Game::LoadContent end");

   return true;
}

void Game::Update(float dt) {
   auto kb = keyboard_->GetState();
   auto mouseState = mouse_->GetState();
   keyTracker_.Update(kb);
   mouseTracker_.Update(mouseState);

   if (keyTracker_.IsKeyReleased(DirectX::Keyboard::Escape)) {
      ExitGame();
   }

   leftHeld_ = mouseTracker_.leftButton == DirectX::Mouse::ButtonStateTracker::HELD;

   UnpressedAll();

   if (leftHeld_) {
      if (data_.gameState == GameState::Play && selectedCell_.IsInBounds()) PressedAround(selectedCell_.x, selectedCell_.y);
      restartButtonPressed_ = PtInRect(&restartButtonRect_, POINT(mouseState.x, mouseState.y));
   }

   if (mouseTracker_.leftButton == DirectX::Mouse::ButtonStateTracker::RELEASED) {
      if (data_.gameState == GameState::Play && selectedCell_.IsInBounds()) ClickAt(selectedCell_.x, selectedCell_.y);
      if (PtInRect(&restartButtonRect_, POINT(mouseState.x, mouseState.y))) Restart();
      restartButtonPressed_ = false;
   }

   if (mouseTracker_.rightButton == DirectX::Mouse::ButtonStateTracker::RELEASED) {
      if (data_.gameState == GameState::Play && selectedCell_.IsInBounds()) MarkAt(selectedCell_.x, selectedCell_.y);
   }
}

void Game::Thread() {
   if (data_.gameState == GameState::Play && data_.started) data_.timer++;
}

void Game::RenderPanel(RECT rect, PanelState state = PanelState::Out) {
   auto width = rect.right - rect.left;
   auto height = rect.bottom - rect.top;

   // lines
   //auto topHorizLine = state == PanelState::In ? &UI::BOTTOM_HORIZONTAL_LINE : &UI::TOP_HORIZONTAL_LINE;
   //auto topHorizLineFlip = state == PanelState::In ? DirectX::SpriteEffects_FlipVertically : DirectX::SpriteEffects_None;
   //auto bottomHorizLine = state == PanelState::In ? &UI::TOP_HORIZONTAL_LINE : &UI::BOTTOM_HORIZONTAL_LINE;
   //auto bottomHorizLineFlip = state == PanelState::In ? DirectX::SpriteEffects_FlipVertically : DirectX::SpriteEffects_None;
   //auto leftVertLine = state == PanelState::In ? &UI::RIGHT_VERTICAL_LINE : &UI::LEFT_VERTICAL_LINE;
   //auto leftVertLineFlip = state == PanelState::In ? DirectX::SpriteEffects_FlipHorizontally : DirectX::SpriteEffects_None;
   //auto rightVertLine = state == PanelState::In ? &UI::LEFT_VERTICAL_LINE : &UI::RIGHT_VERTICAL_LINE;
   //auto rightVertLineFlip = state == PanelState::In ? DirectX::SpriteEffects_FlipHorizontally : DirectX::SpriteEffects_None;
   // corners
   //auto tlc = state == PanelState::In ? &UI::BOTTOM_RIGHT_CORNER : &UI::TOP_LEFT_CORNER;
   //auto tlcFlip = state == PanelState::In ? DirectX::SpriteEffects_FlipBoth : DirectX::SpriteEffects_None;
   //auto trc = state == PanelState::In ? &UI::BOTTOM_LEFT_CORNER : &UI::TOP_RIGHT_CORNER;
   //auto trcFlip = state == PanelState::In ? DirectX::SpriteEffects_FlipBoth : DirectX::SpriteEffects_None;
   //auto blc = state == PanelState::In ? &UI::TOP_RIGHT_CORNER : &UI::BOTTOM_LEFT_CORNER;
   //auto blcFlip = state == PanelState::In ? DirectX::SpriteEffects_FlipBoth : DirectX::SpriteEffects_None;
   //auto brc = state == PanelState::In ? &UI::TOP_LEFT_CORNER : &UI::BOTTOM_RIGHT_CORNER;
   //auto brcFlip = state == PanelState::In ? DirectX::SpriteEffects_FlipBoth : DirectX::SpriteEffects_None;

   // top left corner
   DirectX::XMFLOAT2 tlcAt = { float(rect.left), float(rect.top) };
   auto tlcWidth = UI::TOP_LEFT_CORNER.right - UI::TOP_LEFT_CORNER.left;
   auto tlcHeight = UI::TOP_LEFT_CORNER.bottom - UI::TOP_LEFT_CORNER.top;
   //Draw(tlcAt, tlc, tlcFlip);

   // top right corner
   auto trcWidth = UI::TOP_RIGHT_CORNER.right - UI::TOP_RIGHT_CORNER.left;
   DirectX::XMFLOAT2 trcAt = { rect.left + float(width - trcWidth), float(rect.top) };
   //Draw(trcAt, trc, trcFlip);

   // bottom left corner
   auto blcHeight = UI::BOTTOM_LEFT_CORNER.bottom - UI::BOTTOM_LEFT_CORNER.top;
   DirectX::XMFLOAT2 blcAt = { float(rect.left), rect.top + float(height - blcHeight) };
   //Draw(blcAt, blc, blcFlip);

   // bottom right corner
   auto brcWidth = UI::BOTTOM_RIGHT_CORNER.right - UI::BOTTOM_RIGHT_CORNER.left;
   auto brcHeight = UI::BOTTOM_RIGHT_CORNER.bottom - UI::BOTTOM_RIGHT_CORNER.top;
   DirectX::XMFLOAT2 brcAt = { rect.left + float(width - brcWidth), rect.top + float(height - brcHeight) };
   //Draw(brcAt, brc, brcFlip);

   for (auto x = tlcWidth; x <= width - trcWidth; x++) { // horizontally
      DirectX::XMFLOAT2 at = { rect.left + float(x), float(rect.top) };
      //Draw(at, topHorizLine, topHorizLineFlip);
      at.y = rect.top + height - blcHeight;
      //Draw(at, bottomHorizLine, bottomHorizLineFlip);
   }

   for (auto y = tlcHeight; y <= height - blcHeight; y++) { // vertically
      DirectX::XMFLOAT2 at = { float(rect.left), rect.top + float(y) };
      //Draw(at, leftVertLine, leftVertLineFlip);
      at.x = rect.left + width - trcWidth;
      //Draw(at, rightVertLine, rightVertLineFlip);
   }

   for (auto x = tlcWidth; x <= width - trcWidth - 1; x++) { // fill center
      for (auto y = tlcHeight; y <= height - blcHeight - 1; y++) {
         DirectX::XMFLOAT2 at = { rect.left + float(x), rect.top + float(y) };
         //Draw(at, &UI::BACKGROUND_RECT);
      }
   }
}

void Game::RenderTopPanel() {
   long width, height;
   GetDefaultSize(width, height);

   //textureSpriteBatch_->Begin(
   //   DirectX::DX11::SpriteSortMode::SpriteSortMode_Deferred,
   //   states_->NonPremultiplied(), states_->LinearWrap());

   RECT size = { 0, 0, width, UI::TOP_PANEL_HEIGHT };
   RenderPanel(size);

   RenderMinesNumber();
   RenderRestartButton();
   RenderTimer();

   //textureSpriteBatch_->End();
}

void Game::RenderNumber(DirectX::XMFLOAT2& pos, int number) {
   auto digits = GetDigits(number);

   // indent
   auto indentNumber = UI::MINES_COUNT_CHAR_NUMBER - digits.size();
   for (auto i = 0; i < indentNumber; i++) {
      pos.x += Texture::NUMBER_WIDTH;
   }
   // digits
   for (auto digit : digits) {
      if (digit == '-') {
         DirectX::XMFLOAT2 minusAt = { pos.x, pos.y + Texture::NUMBER_HEIGHT / 2 };
         //Draw(minusAt, &Texture::MINUS, DirectX::Colors::DarkRed);
      }
      else {
         auto rect = Texture::GetDigitRect(digit);
         //Draw(pos, &rect, DirectX::Colors::DarkRed);
      }
      pos.x += Texture::NUMBER_WIDTH;
   }
}

void Game::RenderMinesNumber() {
   int minesAndFlagged = MINES_COUNT - data_.flagged;
   DirectX::XMFLOAT2 at = { 10, 40 };
   RECT size = { at.x, at.y - 10, at.x + Texture::NUMBER_WIDTH * UI::MINES_COUNT_CHAR_NUMBER + 10, at.y + Texture::NUMBER_HEIGHT + 10 };
   RenderPanel(size, PanelState::In);
   RenderNumber(at, minesAndFlagged);
}

void Game::RenderRestartButton() {
   long width, height;
   GetDefaultSize(width, height);
   height = UI::TOP_PANEL_HEIGHT;
   auto buttonWidth = Texture::CELL_WIDTH + 6;
   auto buttonHeight = Texture::CELL_HEIGHT + 6;
   restartButtonRect_ = { width / 2 - buttonWidth / 2 , height / 2 - buttonHeight / 2, width / 2 + buttonWidth / 2, height / 2 + buttonHeight / 2 };
   RenderPanel(restartButtonRect_, restartButtonPressed_ ? PanelState::In : PanelState::Out);
   DirectX::XMFLOAT2 at = { float(restartButtonRect_.left + 3), float(restartButtonRect_.top + 6) };
   //Draw(at, &Texture::MINE_RECT);
}

void Game::RenderTimer() {
   long width, height;
   GetDefaultSize(width, height);
   height = UI::TOP_PANEL_HEIGHT;

   DirectX::XMFLOAT2 at = { width / 3 + width / 3.0f, 40 };
   RECT size = { at.x - 4, at.y - 10, at.x + Texture::NUMBER_WIDTH * UI::MINES_COUNT_CHAR_NUMBER + 10, at.y + Texture::NUMBER_HEIGHT + 10 };
   RenderPanel(size, PanelState::In);
   RenderNumber(at, data_.timer);
}

void Game::RenderGameField() {
   sprite_.Begin();

   for (auto x = 0; x < CELLS_X; x++) {
      for (auto y = 0; y < CELLS_Y; y++) {
         RECT at;
         at.left = x * CELL_WIDTH;
         at.right = at.left + CELL_WIDTH;
         at.top = y * CELL_HEIGHT + UI::TOP_PANEL_HEIGHT;
         at.bottom = at.top + CELL_HEIGHT;
         auto cell = GetCell(x, y);
         if (!cell->opened) {
            auto color = cell->pressed ? DirectX::Colors::Red : DirectX::Colors::White;
            sprite_.Draw(&at, &Texture::CELL_RECT, &color);
            if (data_.gameState == GameState::Defeat && cell->mined) {
               sprite_.Draw(&at, &Texture::MINE_RECT, &DirectX::Colors::White);
            }
            if (cell->IsMarked()) {
               auto paddingX = 6; auto paddingY = 2;
               RECT at;
               at.left = x * CELL_WIDTH + paddingX;
               at.right = at.left + CELL_WIDTH - paddingX * 2;
               at.top = y * CELL_HEIGHT + UI::TOP_PANEL_HEIGHT + paddingY;
               at.bottom = at.top + CELL_HEIGHT - paddingY * 2;
               auto texture = cell->state == RCellState::Flagged ? &Texture::FLAG_RECT : &Texture::QUESTION_MARK_RECT;
               sprite_.Draw(&at, texture);
            }
         }
         else if (cell->mined) {
            sprite_.Draw(&at, &Texture::MINE_RECT);
         }
         else if (cell->minesNear > 0) {
            auto paddingX = NUMBER_WIDTH_HALF;
            auto paddingY = NUMBER_HEIGHT_HALF;
            RECT at;
            at.left = x * CELL_WIDTH + paddingX;
            at.right = at.left + CELL_WIDTH - paddingX * 2;
            at.top = y * CELL_HEIGHT + UI::TOP_PANEL_HEIGHT + paddingY;
            at.bottom = at.top + CELL_HEIGHT - paddingY * 2;
            auto rect = Texture::GetDigitRect(cell->minesNear);
            sprite_.Draw(&at, &rect, &NUMBER_TINTS[cell->minesNear - 1]);
         }
      }
   }

   sprite_.End();
}

void Game::Render() {
   if (d3d_.ctx_ == 0) return;

   d3d_.ctx_->ClearRenderTargetView(d3d_.renderTargetView_.Get(),
      DirectX::Colors::Gray);

   UINT stride = sizeof(VertexPos);
   UINT offset = 0;

   d3d_.ctx_->IASetInputLayout(inputLayout_.Get());
   d3d_.ctx_->IASetVertexBuffers(0, 1, sprite_.vertexBuffer_.GetAddressOf(), &stride, &offset);
   d3d_.ctx_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

   d3d_.ctx_->VSSetShader(vertexShader_.Get(), 0, 0);
   d3d_.ctx_->PSSetShader(pixelShader_.Get(), 0, 0);
   d3d_.ctx_->PSSetShaderResources(0, 1, sprite_.texture_.GetAddressOf());
   d3d_.ctx_->PSSetSamplers(0, 1, samplerState_.GetAddressOf());

   RenderGameField();
   //RenderTopPanel();

   d3d_.swapChain_->Present(1, 0);
}