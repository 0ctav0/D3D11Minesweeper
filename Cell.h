#pragma once

struct Cell {
   bool opened : 1;
   bool flagged : 1;
   bool mined : 1;
   bool pressed : 1;
   // -1 not checked cell
   unsigned char minesNear : 4;
};