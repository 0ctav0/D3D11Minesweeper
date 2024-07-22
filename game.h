#pragma once

#include "DeviceManager.h"
#include "Controller.h"
#include "Cell.h"

// percentage of mines
enum Difficulty {
   Easy = 10,
   Medium = 20,
   Hard = 30,
   Impossible = 40
};

auto constexpr CELLS_X = 10;
auto constexpr CELLS_Y = 8;
int constexpr MINES_COUNT = CELLS_X * CELLS_Y / 100.0f * Difficulty::Easy;

class Game {
public:
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

   HINSTANCE hInstance_;
   HWND hwnd_;
   long width_;
   long height_;

   DeviceManager d3d_;
   Controller cntrl_;

   Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> texture_;
   std::unique_ptr<DirectX::SpriteBatch> textureSpriteBatch_;
   std::unique_ptr<DirectX::CommonStates> states_;
   DirectX::SimpleMath::Vector2 origin_;
   RECT tileRect_;

   std::unordered_map<std::string, Cell> cells_;

   int mouseX_, mouseY_ = 0;
   DirectX::SimpleMath::Vector2 selectedCell_;
};