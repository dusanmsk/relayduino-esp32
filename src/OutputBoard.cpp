
#include "OutputBoard.h"
#include "BoardConfiguration.h"
#include "mgos.h"

static void processTimeoutsCallback(void* outputBoardInstance) {
  ((OutputBoard*)outputBoardInstance)->processTimeouts();
}



OutputBoard::OutputBoard(int boardId, int masterBoardId)  {
  this->boardId = boardId;
  snprintf(this->boardIdString, sizeof(this->boardIdString), "%d/%d", masterBoardId, boardId);
  for(int portId = 0; portId <= NUM_OF_PORTS; portId++ ) {
    output_ports[portId] = new OutputPort();
  }
}

bool OutputBoard::init() {
  // setup i2c 
  LOG(LL_INFO, ("Initializing output board %s", boardIdString));
  if (!(this->mcp_dev = mgos_mcp23017_create(mgos_i2c_get_global(), MCP23017_I2C_ADDRESS_OFFSET + this->boardId, -1))) {
    LOG(LL_ERROR, ("Could not initialize io board %s", boardIdString));
    return false;
  }

  // setup ports
  for(int port=1; port<=16; port++) {
    LOG(LL_DEBUG, ("Setting io board %s pport %d as output", boardIdString, port));
    if(!mgos_mcp23xxx_gpio_set_mode(mcp_dev, port - 1, MGOS_GPIO_MODE_OUTPUT)) {
      return false;
    }
    mgos_mcp23xxx_gpio_write(mcp_dev, port - 1, false);
  }

  // todo odkomentovat a doladit
  // mgos_set_timer(1000, MGOS_TIMER_REPEAT, processTimeoutsCallback, this);

  return true;
}

void OutputBoard::flush() {
  logSettingOutputPorts();
  LOG(LL_DEBUG, ("Sending all data to io ports on board %s", boardIdString));
  for(int port=1; port<=16; port++) {
    int mcpPort = mapLogicalToMCPPort(port);
    if(mcpPort != -1) {
      int value = output_ports[port]->value;
      //LOG(LL_INFO, ("Writting %d to mcp port %d", value, mcpPort));
      mgos_mcp23xxx_gpio_write(mcp_dev, mcpPort, (value > 0));
    }
  }
  LOG(LL_DEBUG, ("Sent all data to io ports on board %s", boardIdString));
}


void OutputBoard::logSettingOutputPorts() {
  // mgos_sys_config_get_debug_level ?
  // todo only if LL_DEBUG is enabled
  char dbg_ports[17];
  for(int port=1; port<=16; port++) {
    dbg_ports[port - 1] = output_ports[port]->value > 0 ? '1' : '0';
  }
  dbg_ports[16] = '\0';
  LOG(LL_DEBUG, ("Setting ports of output board %s to %s", boardIdString, dbg_ports));
}

/**
 * Ports are numbered from 1 to 16
 */
void OutputBoard::setPort(int port, int value) {
  if(port >= 1 && port <= 16) {
    double now = mgos_uptime();
    OutputPort* outputPort = output_ports[port];
    outputPort->value = (value > 0 ? 1 : 0);
    outputPort->timestamp = now;
    LOG(LL_DEBUG, ("Setting port %d to %d on board %s", port, outputPort->value, boardIdString));
    flush();
  }
}


// todo toto segfaultuje
void OutputBoard::processTimeouts() {
  double now = mgos_uptime();
  for(int port = 1; port <= 16; port++ ) {
    OutputPort* outputPort = output_ports[port];
    if(outputPort->value > 0) {
      double portTimeoutMicros = BoardConfiguration::getPortTimeoutMicros(boardId, port);
      // LOG(LL_DEBUG, ("Now is %f, port timestamp is %f, timeout is %f", now, ioPort->timestamp, portTimeoutMicros));
      if(now - outputPort->timestamp > portTimeoutMicros) {
        LOG(LL_DEBUG, ("Board %s timeout on pin %d", boardIdString, port));
        setPort(port, 0);
      }
    }
  }
}



