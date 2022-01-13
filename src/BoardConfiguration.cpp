#include "BoardConfiguration.h"
#include "mgos_config.h"

double BoardConfiguration::getPortTimeoutMicros(int boardId, int portId) {
    // todo get from configuration file per port or return default
    return mgos_sys_config_get_masterboard_out_off_timeout() * 1000;
}

