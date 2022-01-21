
#include "mgos.h"
#include "mgos_rpc.h"
#include "frozen.h"
//#include "mgos_i2c.h"   // todo remove
//#include "mgos_mcp23xxx.h"  // todo remove
//#include "mgos_mcp23xxx_internal.h"  // todo remove
#include "mgos_timers.h"
#include "IOBoard.h"
#include "transport/UdpTransport.h"
#include "constants.h"
#include "MasterBoard.h"
#include <string>
using namespace std;

struct mg_mgr mgr;

static UdpTransport udpTransport;
static MasterBoard masterBoard;

static void initUdpTransport(bool init) {
  if(true) {    // todo config.udp enabled
    udpTransport.init(&masterBoard, init);
    
  }
}

/*
static void debug_timer_callback(void *arg) {
  SendCommand* sendCommand = new SendCommand();
  sendCommand->inputBoard = 4;
  sendCommand->inputPort = 3;
  sendCommand->masterBoard = 2;
  sendCommand->value = 1;
  udpTransport.sendCommand(sendCommand);
  delete sendCommand;
}
*/

static void hello_timer_callback(void *arg) {
  // todo remove
  // masterBoard.logStatistics();
}

static void networkEventHandler(int ev, void *evd, void *arg) {

   switch (ev) {
    case MGOS_NET_EV_DISCONNECTED:
      LOG(LL_INFO, ("%s", "Net disconnected"));
      initUdpTransport(false);
      break;
    case MGOS_NET_EV_CONNECTING:
      LOG(LL_INFO, ("%s", "Net connecting..."));
      break;
    case MGOS_NET_EV_CONNECTED:
      LOG(LL_INFO, ("%s", "Net connected"));
      break;
    case MGOS_NET_EV_IP_ACQUIRED:
      LOG(LL_INFO, ("%s", "Net got IP address"));
      initUdpTransport(true);
      // todo remove
      //mgos_set_timer(3000, MGOS_TIMER_REPEAT, debug_timer_callback, NULL);  
      break;
  }

  (void) evd;
  (void) arg;
}

static void rpc_GetStats_handler(struct mg_rpc_request_info *ri, void *cb_arg, struct mg_rpc_frame_info *fi, struct mg_str args) {
  size_t buf_len = 2048;
  char buf[buf_len];
  struct json_out out = JSON_OUT_BUF(buf, buf_len);
  masterBoard.getStatistics(&out);
  mg_rpc_send_responsef(ri, "%s", buf);
  ri = NULL;
}

static int button1_hold_counter = 0;
static bool goingToReboot = false;
static void button1_timer_handler(void* arg) {
  // handle button
  bool buttonHold = mgos_gpio_read(BUTTON1_PIN) == 0;
  if(buttonHold) {
    button1_hold_counter++;
  } else if (!goingToReboot) {
    button1_hold_counter--;
  }
  if(button1_hold_counter < 0) {
    button1_hold_counter = 0;
  }
  // if hold for a while
  if(button1_hold_counter == 20) {
     goingToReboot = true;
  }
  // now wait to release a button
  if(goingToReboot) {
    if(buttonHold) {
      masterBoard.blinkRedLED();
    } else {
      int oldValue = mgos_sys_config_get_wifi_ap_enable();
      LOG(LL_INFO, ("Turning %s wifi AP and rebooting", oldValue == 1 ? "ON" : "OFF"));
      mgos_sys_config_set_wifi_ap_enable(oldValue > 0 ? 0 : 1);
      mgos_sys_config_save(&mgos_sys_config, false, NULL); 
      mgos_system_restart();
    }
  }
}


enum mgos_app_init_result mgos_app_init(void) {
  LOG(LL_INFO, ("Initializing application v 777"));

  if(!masterBoard.init()) {
    LOG(LL_ERROR, ("Failed to initialize masterboard"));
    return MGOS_APP_INIT_ERROR;  
  }

  mg_mgr_init(&mgr, NULL);
  mgos_event_add_group_handler(MGOS_EVENT_GRP_NET, networkEventHandler, NULL);


  mgos_set_timer(15000, MGOS_TIMER_REPEAT, hello_timer_callback, NULL);

  // setup button1 handler
  mgos_gpio_set_mode(BUTTON1_PIN, MGOS_GPIO_MODE_INPUT);
  mgos_gpio_set_pull(BUTTON1_PIN, MGOS_GPIO_PULL_UP);
  mgos_set_timer(250, MGOS_TIMER_REPEAT, button1_timer_handler, NULL);

  mg_rpc_add_handler(mgos_rpc_get_global(), "Foo.GetStats", "", rpc_GetStats_handler, NULL);

  LOG(LL_INFO, ("Initialization complete"));

  masterBoard.blinkGreenLED();
  
  return MGOS_APP_INIT_SUCCESS;
}



























/*


#define ADDR_BUF_SIZE 256
struct mg_mgr mgr;
char addr[ADDR_BUF_SIZE];
const char *port1 = "udp://4444";

static void ev_handler(struct mg_connection *nc, int ev, void *p, void* p2) {
  struct mbuf *io = &nc->recv_mbuf;
  (void) p;
  if(ev == MG_EV_RECV) {
      string received_message((const char*)io->buf, io->len);
      LOG(LL_INFO, ("Received: %s", received_message.c_str()));
      mbuf_remove(io, io->len);       // Discard message from recv buffer
  }
}

static void timer_cb(void *arg) {
  static bool s_tick_tock = false;
  LOG(LL_INFO,
      ("%s uptime: %.2lf, RAM: %lu, %lu free", (s_tick_tock ? "Tick" : "Tock"),
       mgos_uptime(), (unsigned long) mgos_get_heap_size(),
       (unsigned long) mgos_get_free_heap_size()));
  
  LOG(LL_INFO, ("Foo: %s\n", mgos_sys_config_get_foo()));
  s_tick_tock = !s_tick_tock;
  (void) arg;
}

enum mgos_app_init_result mgos_app_init(void) {
  mgos_set_timer(1000, MGOS_TIMER_REPEAT, timer_cb, NULL);


  mg_mgr_init(&mgr, NULL);
  mg_bind(mgos_get_mgr(), port1, &ev_handler, 0);
//  mg_bind(&mgr, port1, &ev_handler);

  return MGOS_APP_INIT_SUCCESS;

}

*/
