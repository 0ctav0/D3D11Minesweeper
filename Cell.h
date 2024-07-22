#pragma once

struct Cell {
   bool opened, flagged, mined = false;
   // -1 not checked cell
   int minesNear = -1;
};