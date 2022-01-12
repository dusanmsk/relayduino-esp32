#ifndef _COMMAND_H_
#define _COMMAND_H_

/**
 * Represents (parsed) command that was received on any transport (udp packet, mqtt message)
 */
struct ReceivedCommand {
    int masterBoard = -1;
    int outputBoard = -1;
    int outputPort = -1;
    int value = -1;
};

/**
 * Represents command (message) that will be sent from board to configured transports (such as input port events etc...)
 */
struct SendCommand {
    int masterBoard = -1;
    int inputBoard = -1;
    int inputPort = -1;
    int value = -1;
};

class ReceivedCommandProcessor {
    public:
    virtual void processReceivedCommand(ReceivedCommand* command) = 0;
};

class SendCommandProcessor {
    public:
    virtual void sendCommand(SendCommand* command) = 0;
};

#endif
