#pragma once

class DeviceManager {
public:
   bool Init(HWND hwnd, long width, long height);
   float XPixelToRelative(float x);
   float YPixelToRelative(float y);
   DirectX::XMFLOAT3 PixelXMFLOAT3(float x, float y, float z = .5f);

   long width_, height_;

   D3D_FEATURE_LEVEL featureLevel_ = D3D_FEATURE_LEVEL_11_1;
   Microsoft::WRL::ComPtr<ID3D11Device1> device_;
   Microsoft::WRL::ComPtr<ID3D11DeviceContext1> ctx_;

   Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain_;
   Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTargetView_;
};