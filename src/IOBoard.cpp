
#include "IOBoard.h"

int mapLogicalToMCPPort(int logicalPortNumber) {
  switch(logicalPortNumber) {
    case 1: return 8;
    case 2: return 9;
    case 3: return 10;
    case 4: return 11;
    case 5: return 12;
    case 6: return 13;
    case 7: return 14;
    case 8: return 15;
    case 9: return 0;
    case 10: return 1;
    case 11: return 2;
    case 12: return 3;
    case 13: return 4;
    case 14: return 5;
    case 15: return 6;
    case 16: return 7;
  }
  LOG(LL_ERROR, ("Invalid logical port specified (%d)", logicalPortNumber));
  return -1;
}

