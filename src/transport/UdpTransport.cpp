
#include "mgos.h"
#include "UdpTransport.h"
#include "Command.h"

static void handleIncomingSocket(struct mg_connection *connection, int ev, void *p, void *p1) {
  struct mbuf *io = &connection->recv_mbuf;
  (void) p;
  if(ev == MG_EV_RECV) {
      std::string received_message((const char*)io->buf, io->len);
      LOG(LL_DEBUG, ("Received UDP command: %s", received_message.c_str()));
      UdpTransport* udpTransport = (UdpTransport*)p1;
      udpTransport->_processCommand(received_message);
      mbuf_remove(io, io->len);       // Discard message from recv buffer
  }
}


static void handleOutgoingSocket(struct mg_connection *connection, int ev, void *p, void *p1) {
   // sendConnection
   UdpTransport* udpTransport = (UdpTransport*)p1;
   switch(ev){
     case MG_EV_CONNECT:
       LOG(LL_INFO, ("Got outgoing udp connection event: connect"));
       udpTransport->sendConnection = connection;
       break;
     case MG_EV_CLOSE:
       LOG(LL_INFO, ("Got outgoing udp connection event: close"));
       udpTransport->sendConnection = NULL;
       break;
   }

}

/*
    init - true = initialize, false = deinitialize
*/
bool UdpTransport::init(MasterBoard* masterBoard, bool init) {
    if(init) {
        LOG(LL_DEBUG, ("Initializing udp receiver"));

        // if not registered as command processor, do it now
        if(this->masterBoard == NULL) {
            this->masterBoard = masterBoard;
            masterBoard->registerProcessor((SendCommandProcessor*)this);
        }

    // setup listning socket
    char listenPortString[20];
    int listenPort = mgos_sys_config_get_masterboard_transport_udp_listenport();
    snprintf(listenPortString, sizeof(listenPortString), "udp://%d", listenPort);
    if(NULL ==  mg_bind(mgos_get_mgr(), listenPortString, &handleIncomingSocket, (void*)this)) {
        LOG(LL_ERROR, ("Failed to bind to udp port %s", listenPortString));
        return false;
    }

    // setup outgoing socket
    snprintf(udpSendAddress, sizeof(udpSendAddress), "udp://%s:%d", mgos_sys_config_get_masterboard_transport_udp_sendaddress(), mgos_sys_config_get_masterboard_transport_udp_sendport());
    if(NULL ==  mg_connect(mgos_get_mgr(), udpSendAddress, &handleOutgoingSocket, (void*)this)) {
        LOG(LL_ERROR, ("Failed to establish outgoing udp connection"));
        return false;
    }
    LOG(LL_DEBUG, ("UDP transport initialization done"));

    }
    
    // deinitialize (network down)
    else {
        sendConnection = NULL;
    }

    return true;
}

void UdpTransport::_processCommand(std::string command) {
    // todo odstranit blikanie ledkou
    mgos_wdt_feed();
    LOG(LL_INFO, ("received command: %s", command.c_str()));
    int commandsParsed = 0;
    ReceivedCommand* receivedCommand = new ReceivedCommand();

    mgos_wdt_feed();
    int relayBoardId, relayId, relayValue, parsedArgs, parsedMasterBoardId;
    parsedArgs = sscanf(command.c_str(), "om%d b%d r%d %d", &parsedMasterBoardId, &relayBoardId, &relayId, &relayValue);
    if (parsedArgs == 4) {
        receivedCommand->masterBoard = parsedMasterBoardId;
        receivedCommand->outputBoard = relayBoardId;
        receivedCommand->outputPort = relayId;
        receivedCommand->value = relayValue;
        commandsParsed++;
    }
    if(commandsParsed > 0) {
      masterBoard->processReceivedCommand(receivedCommand);
    }
    delete(receivedCommand);
}

void UdpTransport::sendCommand(SendCommand* command) {
    if(sendConnection == NULL) {
        LOG(LL_WARN, ("Outgoing udp connection is not initialized"));
        return;
    }
    LOG(LL_DEBUG, ("Sending udp packet 'im%d b%d i%d %d' to %s", command->masterBoard, command->inputBoard, command->inputPort, command->value, udpSendAddress));
    mg_printf(this->sendConnection, "im%d b%d i%d %d\n", command->masterBoard, command->inputBoard, command->inputPort, command->value);        // todo remove \n
    LOG(LL_DEBUG, ("Sent udp packet 'im%d b%d i%d %d' to %s", command->masterBoard, command->inputBoard, command->inputPort, command->value, udpSendAddress));
}





