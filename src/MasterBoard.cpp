#include <functional>
#include "MasterBoard.h"
#include "transport/Command.h"
#include "mgos.h"
#include <stdio.h>
#include <ctype.h>
#include <sstream>

using namespace std;

static void logAllOutputsCallback(void *arg) {
    ((MasterBoard*) arg)->logAllOutputs();
}

static void processTimeoutsCallback(void *arg) {
    ((MasterBoard*) arg)->processTimeouts();
}

static void connectionLostCallback(void* arg) {
    ((MasterBoard*) arg)->processConnectionLost();
}

MasterBoard::MasterBoard() {
    masterBoardId =  mgos_sys_config_get_masterboard_id();
}

bool MasterBoard::init() {
    mgos_gpio_set_mode(LED_GREEN_PIN, MGOS_GPIO_MODE_OUTPUT);
    mgos_gpio_set_mode(LED_RED_PIN, MGOS_GPIO_MODE_OUTPUT);
    mgos_gpio_write(LED_GREEN_PIN, false);
    mgos_gpio_write(LED_RED_PIN, false);

    this->resetConnectionLostIndicationTimer();
    this->initIOBoards();
    return true;
}

int MasterBoard::getId() {
    return masterBoardId;
}

void MasterBoard::resetConnectionLostIndicationTimer() {
    setRedLed(false);
    mgos_clear_timer(connectionLostCheckTimerId);
    connectionLostCheckTimerId = mgos_set_timer(mgos_sys_config_get_masterboard_out_off_timeout()*1000,
      0,
      connectionLostCallback,
      this);
}

void MasterBoard::processReceivedCommand(ReceivedCommand* command) {
    int ledMode = mgos_sys_config_get_masterboard_led_mode();
    int cmdProcessed = 0;
    resetConnectionLostIndicationTimer();
    if(command->masterBoard == getId()) {
      cmdProcessed++;
      if(command->outputBoard != -1) {
          this->setOutputPort(command->outputBoard, command->outputPort, command->value);
          cmdProcessed++;
      }
    }
    //LOG(LL_DEBUG, ("cmdProcessed=%d, ledMode=%d", cmdProcessed, ledMode));
    if(cmdProcessed >= ledMode) {
      blinkGreenLED();
    }
}

void MasterBoard::setOutputPort(int outputBoardId, int outputPort, int value) {
    if(!(outputBoardId >=0 && outputBoardId <= 7) || outputBoards[outputBoardId] == NULL) {
        LOG(LL_WARN, ("Output board id %d not present", outputBoardId));
        return;
    }
    OutputBoard* outputBoard = this->outputBoards[outputBoardId];
    outputBoard->setPort(outputPort, value);
}


static void turnOffGreenLedCallback(void* arg) {setGreenLed(false);}
void MasterBoard::blinkGreenLED() {
    setGreenLed(true);
    mgos_set_timer(COMMAND_RECV_INDICATION_LED_MSEC, 0, turnOffGreenLedCallback, NULL);
}

static void turnOffRedLedCallback(void* arg) {setRedLed(false);}
void MasterBoard::blinkRedLED() {
    setRedLed(true);
    mgos_set_timer(RED_LED_BLINK_MSEC, 0, turnOffRedLedCallback, NULL);
}

void MasterBoard::getStatistics(struct json_out *out) {
  std::stringstream ss1;
  prepareFoundBoardsString(ss1);
  std::stringstream ss2;
  prepareBoardsTypeString(ss2);

  json_printf(out, "{masterboardId: %d, boards:%Q, btypes:%Q}",
    getId(),
    ss1.str().c_str(),
    ss2.str().c_str()
  );
}


void MasterBoard::prepareFoundBoardsString(std::stringstream& out) {
  for(int i = 0; i < 8; i++) {
    if(presentBoards[i]) {
      out << i;
    } else {
      out << ' ';
    }
  }
}


void MasterBoard::prepareBoardsTypeString(std::stringstream& out) {
  for(int i = 0; i < 8; i++) {
    char ch = '-';
    if(presentBoards[i]) {
      if(outputBoards[i] != NULL) {
        ch = 'O';
      }
      if(inputBoards[i] != NULL) {
        ch = 'I';
      }
    }
    out << ch;
  }
}

void MasterBoard::processConnectionLost() {
    LOG(LL_INFO, ("Connection lost, no command for configured time received. Turning off all outputs."));
    setRedLed(true);
    for(int boardId = 0; boardId < 8; boardId++) {
    OutputBoard* outputBoard = outputBoards[boardId];
    if(outputBoard != NULL) {
      for(int p = 1; p <= 16; p++) {
        outputBoard->setPort(p, 0);
      }
    }
  }
}



/**
 * Returns ['-', 'i', 'o'] for [unconfigured/unknown, input, output] slave board type
 */
char MasterBoard::getBoardType(int boardId) {
  const char* slaveTypes = mgos_sys_config_get_masterboard_slaves();
  if(strnlen(slaveTypes, 9) != 8) {
    LOG(LL_ERROR, ("slave.types config string has not 8 characters"));
    return '-';
  }
  if(boardId < 0 || boardId > 7) {
    LOG(LL_ERROR, ("Illegal board id requested (%d)", boardId));
    return '-';
  }
  char ch = slaveTypes[boardId];
  return tolower(ch);
}

void MasterBoard::initIOBoards() {
  auto inputBoardCallback = std::bind(&MasterBoard::processInputBoardEvent, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
  LOG(LL_INFO, ("Initializing io board instances"));
  for(int boardId = 0; boardId < 8; boardId++) {
    this->presentBoards[boardId] = false;
    if(mgos_mcp23017_create(mgos_i2c_get_global(), MCP23017_I2C_ADDRESS_OFFSET + boardId, -1)) {
      LOG(LL_INFO, ("Found board %d", boardId));
      this->presentBoards[boardId] = true;
    }
  }


  for(int boardId = 0; boardId < 8; boardId++) {
    inputBoards[boardId] = NULL;
    outputBoards[boardId] = NULL;
    if(presentBoards[boardId]) {
      char type = getBoardType(boardId);
      if(type == 'i') {
        InputBoard *inputBoard = new InputBoard(boardId, getId(), inputBoardCallback);
        inputBoard->init();
        inputBoards[boardId] = inputBoard;
        LOG(LL_INFO, ("Created InputBoard instance for board %d", boardId));
      }
      else if(type == 'o') {
        OutputBoard *outputBoard = new OutputBoard(boardId, getId());
        outputBoard->init();
        outputBoards[boardId] = outputBoard;
        LOG(LL_INFO, ("Created OutputBoard instance for board %d", boardId));
      }
    }
  }
  LOG(LL_INFO, ("Done initializing io boards"));
  //logStatistics();
}


// todo mozno nahradit timerom priamo na inputborade, ktory sa inicializuje v init() metode
void MasterBoard::processTimeouts() {
  for(int boardId = 0; boardId < 8; boardId++) {
    OutputBoard* outputBoard = outputBoards[boardId];
    if(outputBoard != NULL) {
      outputBoard->processTimeouts();
    }
  }
}


// todo presunut priamo do input/output board classy
void MasterBoard::logAllOutputs() {
  /*
  char buf[17];
  buf[16] = 0;
  for(int boardId = 0; boardId < 8; boardId++) {
    IOBoard* board = ioBoards[boardId];
    if(board != NULL) {
      for(int portId = 0; portId < 16; portId++) {
        buf[portId] = (board->getPort(portId) > 0 ? '1' : '0');
      }
      LOG(LL_INFO, ("Board %02d status %s", boardId, buf));
    }
  }
  */
}



void MasterBoard::processInputBoardEvent(int boardId, int port, int value) {
    LOG(LL_DEBUG, ("Processing input board %d value %d on port %d", boardId, value, port));
    SendCommand* sendCommand = new SendCommand();
    sendCommand->masterBoard = getId();
    sendCommand->inputBoard = boardId;
    sendCommand->inputPort = port;
    sendCommand->value = value;
    
    // dispatch command to all known processors
    for(auto const& value: sendCommandProcessors) {
      LOG(LL_DEBUG, ("Dispatching ..."));
      value->sendCommand(sendCommand);
    }
    delete(sendCommand);
    
    // todo chcem to? alebo config value? blinkGreenLED();
}


