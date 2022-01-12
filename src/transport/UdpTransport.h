#ifndef _UDP_RECEIVER_H
#define _UDP_RECEIVER_H

#include <string>
#include "mgos.h"
#include "Protocol.h"
#include "../MasterBoard.h"

class UdpTransport : public Protocol, public SendCommandProcessor {
    private:
        MasterBoard* masterBoard;
        char udpSendAddress[128];
        
    public:
        struct mg_connection* sendConnection;
        bool init(MasterBoard* receivedCommandProcessor, bool init);
        void sendCommand(SendCommand* command);
        void _processCommand(std::string command);      // callback handler
};



#endif
