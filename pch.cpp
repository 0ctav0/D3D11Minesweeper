//
// pch.cpp
// Include the standard header and generate the precompiled header.
//

#include "pch.h"

namespace DX {
   unsigned long Now() {
      auto now = std::chrono::system_clock::now();
      return std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
   }
}