//
// pch.h
// Header for standard system include files.
//

#pragma once

#include <winsdkver.h>
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0A00
#endif
#include <sdkddkver.h>

// Use the C++ standard templated min/max
#define NOMINMAX

// DirectX apps don't need GDI
#define NODRAWTEXT
#define NOGDI
#define NOBITMAP

// Include <mcx.h> if you need this
#define NOMCX

// Include <winsvc.h> if you need this
#define NOSERVICE

// WinHelp is deprecated
#define NOHELP

#define WIN32_LEAN_AND_MEAN

#define _USE_MATH_DEFINES

#include <Windows.h>

#include <wrl/client.h>

#include <d3d11_1.h>
#include <dxgi1_2.h>
#include <DirectXMath.h>
#include <DirectXColors.h>

#include <Keyboard.h>
#include <Mouse.h>
#include <CommonStates.h>
#include <SimpleMath.h>
#include <SpriteBatch.h>
#include "WICTextureLoader.h"
#include <Audio.h>

#include <iostream>
#include <fstream>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <exception>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <tuple>
#include <array>
#include <format>
#include <random>
#include <functional>
#include <chrono>
#include <thread>

namespace Log {
   inline std::ofstream file;
   inline void Info(char const* const message) {
      file << "Info: " << message << std::endl;
   }
   inline void Error(char const* const message) {
      file << "Error: " << message << std::endl;
   }
}

namespace DX {
   inline void ThrowIfFailed(HRESULT hr, char const* const message) {
      if (FAILED(hr)) {
         // Set a breakpoint on this line to catch DirectX API errors
         Log::Error(message);
         throw std::exception(message);
      }
   }

   template<typename... Args>
   void Print(wchar_t const* format, Args... args) {
      wchar_t buf[100];
      swprintf_s(buf, format, args...);
      OutputDebugStringW(buf);
   }

   unsigned long Now();
}
