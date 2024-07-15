#include "pch.h"
#include "Controller.h"

bool Controller::Init(HWND hwnd) {
   keyboard_ = std::make_unique<DirectX::Keyboard>();
   mouse_ = std::make_unique<DirectX::Mouse>();
   mouse_->SetWindow(hwnd);

   return true;
}