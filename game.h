#pragma once

#include "DeviceManager.h"
#include "Cell.h"


// percentage of mines
enum Difficulty {
   Easy = 10,
   Medium = 15,
   Hard = 20,
   Impossible = 25
};

enum GameState {
   Play, Win, Defeat
};

auto constexpr CELLS_X = 20;
auto constexpr CELLS_Y = 16;
int constexpr MINES_COUNT = CELLS_X * CELLS_Y / 100.0f * Difficulty::Medium;
auto constexpr NEED_TO_OPEN = CELLS_X * CELLS_Y - MINES_COUNT;

const std::array<DirectX::XMVECTORF32, 8> NUMBER_TINTS = {
   DirectX::Colors::Black,
   DirectX::Colors::Magenta,
   DirectX::Colors::Red,
   DirectX::Colors::Orange,
   DirectX::Colors::AliceBlue,
   DirectX::Colors::Beige,
   DirectX::Colors::DarkOrchid,
   DirectX::Colors::Honeydew,
};

struct Pos {
   int x;
   int y;

   bool IsInBounds() {
      return (x >= 0 && x < CELLS_X && y >= 0 && y < CELLS_Y);
   }
};

class Game {
public:
   ~Game();
   void GetDefaultSize(long& width, long& height);
   bool ExitGame();

   void OnMouseMove();

   bool Init(HINSTANCE hInstance, HWND hwnd);
   bool LoadContent();
   void Update(float dt);
   void Render();

private:
   void InitCells();
   void InitMines(int x, int y);
   Cell* GetCell(int x, int y);
   void OpenAt(int x, int y);
   void IterateAll(std::function<void(Cell* cell)> cb);
   void IterateNear(int originX, int originY, std::function<void(int, int)> cb);
   void ExploreMap(int originX, int originY);
   void OpenNearForced(int originX, int originY);
   void PressedAround(int originX, int originY);
   void UnpressedAll();
   void ClickAt(int x, int y);
   void MarkAt(int x, int y);
   bool IsCellSelected(int x, int y);
   void Defeat();
   void Win();

   std::vector<char> GetDigits(int number);

   void Draw(DirectX::XMFLOAT2 const& pos, RECT const* sourceRectangle, DirectX::FXMVECTOR color, float scaling);
   void RenderPanel(RECT size);
   void RenderTopPanel();
   void RenderMinesNumber();
   void RenderGameField();

   HINSTANCE hInstance_;
   HWND hwnd_;
   long width_;
   long height_;

   DeviceManager d3d_;

   std::unique_ptr<DirectX::Keyboard> keyboard_;
   DirectX::Keyboard::KeyboardStateTracker keyTracker_;

   std::unique_ptr<DirectX::Mouse> mouse_;
   DirectX::Mouse::Mouse::ButtonStateTracker mouseTracker_;
   bool leftHeld_ = false;

   std::unique_ptr<DirectX::AudioEngine> audioEngine_;
   std::unique_ptr<DirectX::SoundEffect> defeatSound_;
   std::unique_ptr<DirectX::SoundEffect> winSound_;

   Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> texture_;
   std::unique_ptr<DirectX::SpriteBatch> textureSpriteBatch_;
   std::unique_ptr<DirectX::CommonStates> states_;
   DirectX::SimpleMath::Vector2 origin_;

   std::array<std::array<Cell, CELLS_Y>, CELLS_X> cells_ = {};

   GameState gameState_ = GameState::Play;
   UINT opened_ = 0;
   UINT flagged_ = 0;
   bool hasOpened_ = false;
   Pos selectedCell_ = {};
};