#include "pch.h"
#include "DeviceManager.h"

bool DeviceManager::Init(HWND hwnd, long width, long height) {
   using namespace Microsoft::WRL;
   Log::Info("DeviceManager::Init start");
   width_ = width;
   height_ = height;
   D3D_FEATURE_LEVEL featureLevels[] = {
     D3D_FEATURE_LEVEL_11_1,
     D3D_FEATURE_LEVEL_11_0,
   };



   unsigned int creationFlags = 0;

#ifdef _DEBUG
   creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif


   unsigned int driver = 0;

   ComPtr<ID3D11Device> device;
   ComPtr<ID3D11DeviceContext> context;

   DX::ThrowIfFailed(D3D11CreateDevice(
      nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, creationFlags,
      featureLevels, static_cast<UINT>(std::size(featureLevels)), D3D11_SDK_VERSION,
      device.GetAddressOf(), &featureLevel_, context.GetAddressOf()),
      "Failed to create the Direct3D device!");

   DX::ThrowIfFailed(device.As(&device_), "Failed to set device");
   DX::ThrowIfFailed(context.As(&ctx_), "Failed to set context");

   ComPtr<IDXGIDevice1> dxgiDevice;
   DX::ThrowIfFailed(device_.As(&dxgiDevice));

   ComPtr<IDXGIAdapter> dxgiAdapter;
   DX::ThrowIfFailed(dxgiDevice->GetAdapter(dxgiAdapter.GetAddressOf()));

   ComPtr<IDXGIFactory2> dxgiFactory;
   DX::ThrowIfFailed(
      dxgiAdapter->GetParent(IID_PPV_ARGS(dxgiFactory.GetAddressOf())));

   DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
   swapChainDesc.Width = width;
   swapChainDesc.Height = height;
   swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
   swapChainDesc.SampleDesc.Count = 1;
   swapChainDesc.SampleDesc.Quality = 0;
   swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
   swapChainDesc.BufferCount = 1;
   swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
   swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

   DXGI_SWAP_CHAIN_FULLSCREEN_DESC fsSwapChainDesc = {};
   fsSwapChainDesc.Windowed = TRUE;

   DX::ThrowIfFailed(dxgiFactory->CreateSwapChainForHwnd(
      device_.Get(), hwnd, &swapChainDesc, &fsSwapChainDesc, nullptr,
      swapChain_.ReleaseAndGetAddressOf()),
      "Failed to create the swap chain");

   ComPtr<ID3D11Texture2D> backBufferTexture;
   DX::ThrowIfFailed(swapChain_->GetBuffer(0, IID_PPV_ARGS(backBufferTexture.GetAddressOf())),
      "Failed to get the swap chain back buffer!");

   DX::ThrowIfFailed(device_->CreateRenderTargetView(backBufferTexture.Get(), 0,
      renderTargetView_.GetAddressOf()),
      "Failed to create the render target view !");

   ctx_->OMSetRenderTargets(1, renderTargetView_.GetAddressOf(), nullptr);

   D3D11_VIEWPORT viewport = {};
   viewport.Width = static_cast<float>(width);
   viewport.Height = static_cast<float>(height);
   viewport.MinDepth = 0.0f;
   viewport.MaxDepth = 1.0f;

   ctx_->RSSetViewports(1, &viewport);

   Log::Info("DeviceManager::Init end");

   return true;
}

float DeviceManager::XPixelToRelative(float x)
{
   return 2.0f * x / width_ - 1;
}

float DeviceManager::YPixelToRelative(float y)
{
   return -2.0f * y / height_ + 1;
}

DirectX::XMFLOAT3 DeviceManager::PixelXMFLOAT3(float x, float y, float z)
{
   return DirectX::XMFLOAT3(XPixelToRelative(x), YPixelToRelative(y), z);
}

void DeviceManager::LoadShader(LPCWSTR fileName, Microsoft::WRL::ComPtr<ID3DBlob>& blob, ID3D11VertexShader** vs, ID3D11PixelShader** ps)
{
   DX::ThrowIfFailed(D3DCompileFromFile(fileName, nullptr, nullptr, "VS_Main", "vs_4_1", 0, 0, blob.GetAddressOf(), nullptr), "Failed to compile a shader from a file");

   DX::ThrowIfFailed(device_->CreateVertexShader(blob.Get()->GetBufferPointer(), blob.Get()->GetBufferSize(), nullptr, vs), "Failed to create vertex shader");

   DX::ThrowIfFailed(D3DCompileFromFile(fileName, nullptr, nullptr, "PS_Main", "ps_4_1", 0, 0, blob.GetAddressOf(), nullptr), "Failed to compile a shader from a file");

   DX::ThrowIfFailed(device_->CreatePixelShader(blob.Get()->GetBufferPointer(), blob.Get()->GetBufferSize(), nullptr, ps), "Failed to create pixel shader");
}
