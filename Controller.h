#pragma once

class Controller {
public:
   bool Init(HWND hwnd);

   std::unique_ptr<DirectX::Keyboard> keyboard_;
   std::unique_ptr<DirectX::Mouse> mouse_;
};
