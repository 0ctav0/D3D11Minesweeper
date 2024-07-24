#pragma once

#include "DeviceManager.h"
#include "Controller.h"
#include "Cell.h"

struct Pos {
   int x;
   int y;
};

// percentage of mines
enum Difficulty {
   Easy = 10,
   Medium = 20,
   Hard = 30,
   Impossible = 40
};

enum GameState {
   Play, Win, Defeat
};

auto constexpr CELLS_X = 10;
auto constexpr CELLS_Y = 8;
int constexpr MINES_COUNT = CELLS_X * CELLS_Y / 100.0f * Difficulty::Medium;
auto constexpr NEED_TO_OPEN = CELLS_X * CELLS_Y - MINES_COUNT;

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
   void InitMines();
   Cell* GetCell(int x, int y);
   void OpenAt(int x, int y);
   void IterateNear(int originX, int originY, std::function<void(int, int)> cb);
   void ExploreMap(int originX, int originY);
   void FlagAt(int x, int y);
   bool IsCellSelected(int x, int y);
   void Defeat();
   void Win();

   HINSTANCE hInstance_;
   HWND hwnd_;
   long width_;
   long height_;

   DeviceManager d3d_;
   Controller cntrl_;
   DirectX::Keyboard::KeyboardStateTracker keyTracker_;


   Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> texture_;
   std::unique_ptr<DirectX::SpriteBatch> textureSpriteBatch_;
   std::unique_ptr<DirectX::CommonStates> states_;
   DirectX::SimpleMath::Vector2 origin_;
   RECT tileRect_;

   std::array<std::array<Cell, CELLS_Y>, CELLS_X> cells_ = {};

   GameState gameState_ = GameState::Play;
   unsigned opened_ = 0;
   Pos selectedCell_ = {};
};