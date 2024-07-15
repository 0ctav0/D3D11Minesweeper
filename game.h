#pragma once

#include "DeviceManager.h"
#include "Controller.h"
#include "Cell.h"

#define CELLS_X 10
#define CELLS_Y 8
#define CELL_WIDTH 64
#define CELL_HEIGHT 64
#define TEXTURE_FILENAME L"img/texture.png"

class Game {
public:
   void GetDefaultSize(long& width, long& height);
   bool ExitGame();

   void OnMouseMove();
   void OnMouseLDown();
   void OnMouseRDown();

   bool Init(HINSTANCE hInstance, HWND hwnd);
   bool LoadContent();
   void Update(float dt);
   void Render();

private:
   HINSTANCE hInstance_;
   HWND hwnd_;
   long width_;
   long height_;

   DeviceManager d3d_;
   Controller cntrl_;

   Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> texture_;
   std::unique_ptr<DirectX::SpriteBatch> textureSpriteBatch_;
   std::unique_ptr<DirectX::CommonStates> states_;
   DirectX::SimpleMath::Vector2 screenPos_;
   DirectX::SimpleMath::Vector2 origin_;
   RECT tileRect_;

   std::unordered_map<std::string, Cell> cells_;

   int mouseX_, mouseY_ = 0;
   DirectX::SimpleMath::Vector2 selectedCell_;
};