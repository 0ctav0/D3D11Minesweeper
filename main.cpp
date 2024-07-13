#include "pch.h"
#include "game.h"


LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

namespace {
   std::unique_ptr<Game> game;
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE prevInstance, LPWSTR cmdLine,
   int cmdShow) {
   UNREFERENCED_PARAMETER(prevInstance);
   UNREFERENCED_PARAMETER(cmdLine);

   game = std::make_unique<Game>();

   WNDCLASSEX wndClass = { 0 };
   wndClass.cbSize = sizeof(WNDCLASSEX);
   wndClass.style = CS_HREDRAW | CS_VREDRAW;
   wndClass.lpfnWndProc = WndProc;
   wndClass.hInstance = hInstance;
   wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
   wndClass.hIcon = LoadIcon(hInstance, 0);
   wndClass.hIconSm = LoadIcon(hInstance, 0);
   wndClass.hbrBackground = (HBRUSH)COLOR_WINDOW + 1;
   wndClass.lpszMenuName = NULL;
   wndClass.lpszClassName = L"D3D11Minesweeper";

   if (!RegisterClassEx(&wndClass)) return -1;

   long width;
   long height;
   game->GetDefaultSize(width, height);

   RECT rc = { 0, 0, width, height };
   AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
   HWND hwnd = CreateWindowExW(0, L"D3D11Minesweeper", L"Minesweeper",
      WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
      rc.right - rc.left, rc.bottom - rc.top, nullptr,
      nullptr, hInstance, nullptr);

   if (!hwnd) return -1;

   ShowWindow(hwnd, cmdShow);

   bool result = game->Init(hInstance, hwnd);

   if (result == false) return -1;

   MSG msg = {};
   while (msg.message != WM_QUIT) {
      if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
         TranslateMessage(&msg);
         DispatchMessage(&msg);
      }
      else {
         game->Update(0.0f);
         game->Render();
      }
   }

   return static_cast<int>(msg.wParam);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam,
   LPARAM lParam) {
   PAINTSTRUCT paintStruct;
   HDC hDC;

   switch (message) {
   case WM_PAINT:
      hDC = BeginPaint(hwnd, &paintStruct);
      EndPaint(hwnd, &paintStruct);
      break;
   case WM_DESTROY:
      PostQuitMessage(0);
      break;
   case WM_SETFOCUS:

      break;
   default:
      return DefWindowProc(hwnd, message, wParam, lParam);
   }

   return 0;
}