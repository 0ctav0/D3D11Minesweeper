#pragma once

enum PanelState : BYTE {
   In, Out
};

typedef struct RECT1 : RECT {
   long Width() {
      return right - left;
   }
   long Height() {
      return bottom - top;
   }
   bool IsInside(RECT1 const& other) {
      return left >= other.left && top >= other.top &&
         right <= other.right && bottom <= other.bottom;
   }
   bool NotIntersects(RECT1 const& other) {
      return left < other.right &&
         right > other.left &&
         top < other.bottom &&
         bottom > other.top;
   }
   bool Intersects(RECT1 const& other) {
      return NotIntersects(other);
   }
   template <typename T>
   void MoveX(T value) {
      left += value;
      right += value;
   }
   template <typename T>
   void MoveY(T value) {
      top += value;
      bottom += value;
   }
};

struct VertexPos {
   DirectX::XMFLOAT3 pos;
   DirectX::XMFLOAT4 color;
   DirectX::XMFLOAT2 tex0;
};

// FlipBoth contains FlipHorizontally and FlipVertically
enum Mirror : BYTE {
   None = 0b0, FlipHorizontally = 0b1, FlipVertically = 0b10, FlipBoth = 0b11
};