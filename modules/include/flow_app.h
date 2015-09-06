#ifndef FLOW_APP_H
#define FLOW_APP_H

#include "c_types.h"
#include "flash_param.h"

#define SIG_UART 0
#define SIG_MQTT 1

#define flowTaskPrio 0
#define flowTaskQueueLen 64

void ICACHE_FLASH_ATTR initFlow(flash_param_t* flashParam);

#endif
