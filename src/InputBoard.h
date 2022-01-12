/**
  TODO doc
**/



#ifndef _INPUT_BOARD_H
#define _INPUT_BOARD_H

#include "mgos.h"
#include "mgos_i2c.h"   // todo remove
#include "mgos_mcp23xxx.h"  // todo remove
#include "mgos_mcp23xxx_internal.h"  // todo remove
#include <functional>

#include "IOBoard.h"


typedef std::function<void (int boardId, int port, int value)> InputBoardCallback;

class InputBoard {

private:
  char boardIdString[32];                    // for logger
  int boardId;
  struct mgos_mcp23xxx * mcp_dev;
  InputBoardCallback inputBoardCallback;
  void* masterboard;

  int debounce_counters[17];            // counters for reading input pins. Used to debounce inputs.
  int last_read_values[17];             // remembered (and debounced) values of pins after last read
  const int debounce_window_size = 6;   // how much identical reads must be taken for input pin to say its value has changed

public:
  InputBoard(int boardId, int masterBoardId, InputBoardCallback callback);
  bool init();
  void readInputs();

};




#endif
