#include "pch.h"
#include "Game.h"

bool Game::Init(HINSTANCE hInstance, HWND hwnd) {

   hInstance_ = hInstance;
   hwnd_ = hwnd;

   RECT dimensions;
   GetClientRect(hwnd, &dimensions);

   auto width = dimensions.right - dimensions.left;
   auto height = dimensions.bottom - dimensions.top;

   D3D_FEATURE_LEVEL featureLevels[] = {
       D3D_FEATURE_LEVEL_11_1,
       D3D_FEATURE_LEVEL_11_0,
   };

   DXGI_SWAP_CHAIN_DESC swapChainDesc;
   ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));
   swapChainDesc.BufferCount = 1;
   swapChainDesc.BufferDesc.Width = width;
   swapChainDesc.BufferDesc.Height = height;
   swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
   swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
   swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;

   swapChainDesc.SampleDesc.Count = 1;
   swapChainDesc.SampleDesc.Quality = 0;

   swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
   swapChainDesc.OutputWindow = hwnd;
   swapChainDesc.Windowed = TRUE;
   swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
   swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

   unsigned int creationFlags = 0;

#ifdef _DEBUG
   creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

   HRESULT hr;
   unsigned int driver = 0;

   Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain;
   Microsoft::WRL::ComPtr<ID3D11Device> device;
   Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;

   hr = D3D11CreateDeviceAndSwapChain(
      nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, creationFlags,
      featureLevels, static_cast<UINT>(std::size(featureLevels)),
      D3D11_SDK_VERSION, &swapChainDesc, swapChain.GetAddressOf(),
      device.GetAddressOf(), &featureLevel_, context.GetAddressOf());

   DX::ThrowIfFailed(hr, "Failed to create the Direct3D device!");

   DX::ThrowIfFailed(device.As(&d3dDevice_));
   DX::ThrowIfFailed(context.As(&d3dContext_));
   DX::ThrowIfFailed(swapChain.As(&swapChain_));

   ID3D11Texture2D* backBufferTexture;
   hr = swapChain_->GetBuffer(0, _uuidof(ID3D11Texture2D),
      (LPVOID*)&backBufferTexture);

   DX::ThrowIfFailed(hr, "Failed to get the swap chain back buffer!");

   hr = d3dDevice_->CreateRenderTargetView(backBufferTexture, 0,
      renderTargetView_.GetAddressOf());

   if (backBufferTexture) backBufferTexture->Release();

   DX::ThrowIfFailed(hr, "Failed to create the render target view !");

   d3dContext_->OMSetRenderTargets(1, renderTargetView_.GetAddressOf(), nullptr);

   D3D11_VIEWPORT viewport;
   viewport.Width = static_cast<float>(width);
   viewport.Height = static_cast<float>(height);
   viewport.TopLeftX = 0.0f;
   viewport.TopLeftY = 0.0f;

   d3dContext_->RSSetViewports(1, &viewport);

   return true;
}

void Game::Update(float dt) {}
void Game::Render() {
   if (d3dContext_ == 0) return;

   d3dContext_->ClearRenderTargetView(renderTargetView_.Get(),
      DirectX::Colors::Aqua);

   swapChain_->Present(1, 0);
}