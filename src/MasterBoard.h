#ifndef _MASTER_BOARD_H
#define _MASTER_BOARD_H

#define I2C_SDA_PIN    13
#define I2C_SCL_PIN    16
#define LED_RED_PIN    32
#define LED_GREEN_PIN  33
#define BUTTON1_PIN    34

#define COMMAND_RECV_INDICATION_LED_MSEC 50
#define RED_LED_BLINK_MSEC 150

#include <vector>
#include <sstream>

#include "frozen.h"
#include "InputBoard.h"
#include "OutputBoard.h"
#include "transport/Command.h"

static inline void setRedLed(bool value) {
    mgos_gpio_write(LED_RED_PIN, value);
}

static inline void setGreenLed(bool value) {
    mgos_gpio_write(LED_GREEN_PIN, value);
}


class MasterBoard : public ReceivedCommandProcessor {

    private:
        std::vector<SendCommandProcessor*> sendCommandProcessors;
        int masterBoardId;
        OutputBoard* outputBoards[8];
        InputBoard* inputBoards[8];
        bool presentBoards[8];
        int connectionLostCheckTimerId;
        long readCount;

        void resetConnectionLostIndicationTimer();
        void prepareBoardsTypeString(std::stringstream&);
        void prepareFoundBoardsString(std::stringstream&);
        char getBoardType(int boardId);
        void initIOBoards();
        void setOutputPort(int outputBoard, int outputPort, int value);
        
    public:
        MasterBoard();
        bool init();
        int getId();
        void blinkGreenLED();
        void blinkRedLED();
        void readInputBoards();
        void processTimeouts();
        void logAllOutputs();
        void getStatistics(struct json_out *out);
        void registerProcessor(SendCommandProcessor* processor) {
            sendCommandProcessors.push_back(processor);
            LOG(LL_DEBUG, ("Registered SendCommandProcessor, count is %d", sendCommandProcessors.size()));
        }

        void processReceivedCommand(ReceivedCommand* command);
        void processConnectionLost();
        void processInputBoardEvent(int boardId, int port, int value);

};

#endif
