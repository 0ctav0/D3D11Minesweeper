#pragma once

using DirectX::Mouse;
using DirectX::Keyboard;

class Controller {
public:
   bool Init(HWND hwnd);
   void BeforeUpdate();
   void AfterUpdate();

   Mouse::State* GetMouseState();
   template <typename MemberType>
   bool MouseDown(MemberType Mouse::State::* memberName);
   template <typename MemberType>
   bool MouseReleased(MemberType Mouse::State::* memberName);

   Keyboard::State* GetKeyboardState();

private:
   std::unique_ptr<Keyboard> keyboard_;
   Keyboard::State currKeyboardState_ = {};
   Keyboard::State prevKeyboardState_ = {};

   std::unique_ptr<Mouse> mouse_;
   Mouse::State currMouseState_ = {};
   Mouse::State prevMouseState_ = {};

};

template<typename MemberType>
bool Controller::MouseReleased(MemberType Mouse::State::* memberName) {
   return (prevMouseState_.*memberName) && !(currMouseState_.*memberName);
}
