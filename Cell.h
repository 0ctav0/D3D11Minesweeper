#pragma once
#include <winnt.h>

enum RCellState : BYTE {
   Still,
   Flagged,
   Questioned,
};

struct Cell {
   bool opened : 1;
   RCellState state : 2;
   bool mined : 1;
   bool pressed : 1;
   BYTE minesNear : 4;

   bool IsMarked() {
      return state == RCellState::Flagged || state == RCellState::Questioned;
   }

   void ToggleState() {
      state = state == RCellState::Still ? RCellState::Flagged : state == RCellState::Flagged ? RCellState::Questioned : RCellState::Still;
   }
};