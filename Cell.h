#pragma once

struct Cell {
   bool opened = false, flagged = false, mined = false;
   // -1 not checked cell
   int minesNear = -1;
};