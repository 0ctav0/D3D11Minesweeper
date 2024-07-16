#include "pch.h"
#include "Controller.h"

bool Controller::Init(HWND hwnd) {
   keyboard_ = std::make_unique<DirectX::Keyboard>();
   mouse_ = std::make_unique<DirectX::Mouse>();
   mouse_->SetWindow(hwnd);

   return true;
}

void Controller::BeforeUpdate() {
   currMouseState_ = mouse_->GetState();
   currKeyboardState_ = keyboard_->GetState();
}

void Controller::AfterUpdate() {
   memcpy(&prevMouseState_, &currMouseState_, sizeof(currMouseState_));
   memcpy(&prevKeyboardState_, &currKeyboardState_, sizeof(currKeyboardState_));
}

Mouse::State* Controller::GetMouseState() {
   return &currMouseState_;
}

Keyboard::State* Controller::GetKeyboardState() {
   return &currKeyboardState_;
}

