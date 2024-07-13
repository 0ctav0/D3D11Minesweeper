#pragma once

class Game {
 public:
  bool Init(HINSTANCE hInstance, HWND hwnd);
  void Update(float dt);
  void Render();

 private:
  HINSTANCE hInstance_;
  HWND hwnd_;

  D3D_FEATURE_LEVEL featureLevel_;
  Microsoft::WRL::ComPtr<ID3D11Device1> d3dDevice_;
  Microsoft::WRL::ComPtr<ID3D11DeviceContext1> d3dContext_;

  Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain_;
  Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTargetView_;
};