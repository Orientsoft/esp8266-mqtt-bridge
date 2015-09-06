#ifndef UART_APP_H
#define UART_APP_H

#include "c_types.h"

void ICACHE_FLASH_ATTR initUart(flash_param_t* flashParam);
void ICACHE_FLASH_ATTR uartTask(os_event_t* events);

#endif
