#pragma once

#define CELLS_X 10
#define CELLS_Y 8
#define CELL_WIDTH 64
#define CELL_HEIGHT 64
#define CELL_FILENAME L"img/cell-64.png"

class Game {
public:
   void GetDefaultSize(long& width, long& height);
   bool Init(HINSTANCE hInstance, HWND hwnd);
   bool LoadContent();
   void Update(float dt);
   void Render();

private:
   HINSTANCE hInstance_;
   HWND hwnd_;
   long width_;
   long height_;

   D3D_FEATURE_LEVEL featureLevel_;
   Microsoft::WRL::ComPtr<ID3D11Device1> d3dDevice_;
   Microsoft::WRL::ComPtr<ID3D11DeviceContext1> d3dContext_;

   Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain_;
   Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTargetView_;

   Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cellTexture_;
   std::unique_ptr<DirectX::SpriteBatch> cellSpriteBatch_;
   std::unique_ptr<DirectX::CommonStates> states_;
   DirectX::SimpleMath::Vector2 screenPos_;
   DirectX::SimpleMath::Vector2 origin_;
   RECT tileRect_;
};