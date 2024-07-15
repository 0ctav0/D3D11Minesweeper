#include "pch.h"
#include "Game.h"

namespace Texture {
   RECT CELL_RECT = { 0, 0, CELL_WIDTH, CELL_HEIGHT };
}

void Game::GetDefaultSize(long& width, long& height) {
   width = CELLS_X * CELL_WIDTH;
   height = CELLS_Y * CELL_HEIGHT;
}

bool Game::Init(HINSTANCE hInstance, HWND hwnd) {

   hInstance_ = hInstance;
   hwnd_ = hwnd;

   RECT dimensions;
   GetClientRect(hwnd, &dimensions);

   width_ = dimensions.right - dimensions.left;
   height_ = dimensions.bottom - dimensions.top;

   D3D_FEATURE_LEVEL featureLevels[] = {
       D3D_FEATURE_LEVEL_11_1,
       D3D_FEATURE_LEVEL_11_0,
   };

   DXGI_SWAP_CHAIN_DESC swapChainDesc;
   ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));
   swapChainDesc.BufferCount = 1;
   swapChainDesc.BufferDesc.Width = width_;
   swapChainDesc.BufferDesc.Height = height_;
   swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
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

   D3D11_VIEWPORT viewport = {};
   viewport.Width = static_cast<float>(width_);
   viewport.Height = static_cast<float>(height_);

   d3dContext_->RSSetViewports(1, &viewport);

   return LoadContent();
}

bool Game::LoadContent() {
   textureSpriteBatch_ = std::make_unique<DirectX::DX11::SpriteBatch>(d3dContext_.Get());
   states_ = std::make_unique<DirectX::DX11::CommonStates>(d3dDevice_.Get());

   Microsoft::WRL::ComPtr<ID3D11Resource> resource;
   DX::ThrowIfFailed(
      DirectX::CreateWICTextureFromFile(d3dDevice_.Get(), TEXTURE_FILENAME, resource.GetAddressOf(),
         texture_.ReleaseAndGetAddressOf()));

   Microsoft::WRL::ComPtr<ID3D11Texture2D> cell;
   DX::ThrowIfFailed(resource.As(&cell));

   CD3D11_TEXTURE2D_DESC cellDesc;
   cell->GetDesc(&cellDesc);

   origin_.x = 0;
   origin_.y = 0;

   tileRect_.left = 0;
   tileRect_.right = cellDesc.Width * CELLS_X;
   tileRect_.top = 0;
   tileRect_.bottom = cellDesc.Height * CELLS_Y;

   screenPos_.x = 0;
   screenPos_.y = 0;

   return true;
}

void Game::Update(float dt) {}

void Game::Render() {
   if (d3dContext_ == 0) return;

   d3dContext_->ClearRenderTargetView(renderTargetView_.Get(),
      DirectX::Colors::Gray);

   textureSpriteBatch_->Begin(DirectX::DX11::SpriteSortMode::SpriteSortMode_Deferred, states_->NonPremultiplied(), states_->LinearWrap());

   for (auto x = 0; x < CELLS_X; x++) {
      for (auto y = 0; y < CELLS_Y; y++) {
         DirectX::XMFLOAT2 at = { float(x * CELL_WIDTH), float(y * CELL_HEIGHT) };
         textureSpriteBatch_->Draw(texture_.Get(), at, &Texture::CELL_RECT, DirectX::Colors::White, 0.f, origin_);
      }
   }

   textureSpriteBatch_->End();

   swapChain_->Present(1, 0);
}