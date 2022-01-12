
#include "InputBoard.h"
#include "BoardConfiguration.h"
#include "mgos.h"


static void readInputsCallback(void* inputBoardInstance) {
  ((InputBoard*)inputBoardInstance)->readInputs();
}

InputBoard::InputBoard(int boardId, int masterBoardId, InputBoardCallback callback)  {
  this->boardId = boardId;
  this->inputBoardCallback = callback;
  snprintf(this->boardIdString, sizeof(this->boardIdString), "%d/%d", masterBoardId, boardId);
  memset(&debounce_counters, 0, sizeof(debounce_counters));
  memset(&last_read_values, 0, sizeof(last_read_values));
}

bool InputBoard::init() {
  // setup i2c 
  LOG(LL_INFO, ("Initializing input board %s", boardIdString));
  if (!(this->mcp_dev = mgos_mcp23017_create(mgos_i2c_get_global(), MCP23017_I2C_ADDRESS_OFFSET + this->boardId, -1))) {
    LOG(LL_ERROR, ("Could not initialize io board %s", boardIdString));
    return false;
  }

  // setup ports
  for(int port=1; port<=16; port++) {
    LOG(LL_DEBUG, ("Setting io board %s port %d as input/pullup", boardIdString, port));
    if(!mgos_mcp23xxx_gpio_set_mode(mcp_dev, port - 1, MGOS_GPIO_MODE_INPUT)) {
      LOG(LL_ERROR, ("gpio set mode failed"));
      return false;
    }
    if(!mgos_mcp23xxx_gpio_set_pull(mcp_dev, port - 1, MGOS_GPIO_PULL_UP)) {
      LOG(LL_ERROR, ("gpio set pull failed"));
      return false;
    }
  }

  mgos_set_timer(1, MGOS_TIMER_REPEAT, readInputsCallback, this);
  LOG(LL_DEBUG, ("Initialized read timer on inputboard %d", boardId));

  // todo something else?

  LOG(LL_INFO, ("Initialized inputboard %d", boardId));
  return true;
}

/**
 * Read input pins and call callback when pin value has changed
 */
void InputBoard::readInputs() {
  uint16_t _gpio = 0;
  if (!mgos_i2c_read_reg_n(mcp_dev->i2c, mcp_dev->i2caddr, MCP23XXX_REG_GPIO * mcp_dev->_w, mcp_dev->_w, (uint8_t *)&_gpio)) {
    LOG(LL_ERROR,("Failed to read inputs on board %s", boardIdString));
  }

  /*
    Debouncing:
    Whenever '1' is read on pin, counter for that pin is increased. When 0 is read, it is decreased.
    When counter reach its maximum value (window size), pin is considered as '1', until counter will not reach 0 again.
  */
  for(int port = 1; port <= 16; port++) {
      int counter = debounce_counters[port];
      if(_gpio & 1) {
        counter--;
      } else {
        counter++;
      }
      if(counter > debounce_window_size) {
        counter = debounce_window_size;
      }
      if(counter < 0) {
        counter = 0;
      }
      debounce_counters[port] = counter;
      _gpio >>= 1;

      // process changes
      int portValue = last_read_values[port];
      if(portValue == 0 && counter == debounce_window_size) {  // 0 to 1
        portValue = 1;
        LOG(LL_DEBUG, ("Port %d value %d on board %s", port, portValue, boardIdString));
        inputBoardCallback(boardId, port, portValue);
     } else if(portValue == 1 && counter == 0) {  // 1 to 0
        portValue = 0;
        LOG(LL_DEBUG, ("Port %d value %d on board %s", port, portValue, boardIdString));
        inputBoardCallback(boardId, port, portValue);
     }
     last_read_values[port] = portValue;

     // todo process periodic resends of input values
  }

}
