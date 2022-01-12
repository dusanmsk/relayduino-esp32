#ifndef _IO_BOARD_H
#define _IO_BOARD_H

#include "mgos.h"

static const int MCP23XXX_REG_GPIO      = 9;
static const int NUM_OF_PORTS = 16;
static const int MCP23017_I2C_ADDRESS_OFFSET = 32;


/*
* Due to difference between port numbers documented on ioboard and port numbers of mcp23xxx the port number has to be remapped to mcp numbering.
* board port 1 = mcp port 9
* board port 8 = mcp port 16
* board port 9 = mcp port 1
* board port 16 = mcp port 8
*/ 
int mapLogicalToMCPPort(int logicalPortNumber);

#endif
