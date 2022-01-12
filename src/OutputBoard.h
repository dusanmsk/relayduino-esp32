/**
  TODO doc
**/



#ifndef _OUTPUT_BOARD_H
#define _OUTPUT_BOARD_H

#include "mgos.h"
#include "mgos_i2c.h"   // todo remove
#include "mgos_mcp23xxx.h"  // todo remove
#include "mgos_mcp23xxx_internal.h"  // todo remove

#include <functional>

#include "IOBoard.h"


typedef struct {
  int value = 0;
  double timestamp = 0;
} OutputPort;


class OutputBoard {

private:
  char boardIdString[32];                    // for logger
  int boardId;
  OutputPort* output_ports[NUM_OF_PORTS+1];
  struct mgos_mcp23xxx * mcp_dev;

  void flush();
  void logSettingOutputPorts();

public:
  OutputBoard(int boardId, int masterBoardId);
  void setPort(int port, int value);
  void processTimeouts();
  bool init();
  const int MCP23017_I2C_ADDRESS_OFFSET = 32;

};




#endif
