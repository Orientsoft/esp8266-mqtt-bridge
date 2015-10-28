#include "ets_sys.h"
#include "os_type.h"
#include "osapi.h"
#include "mem.h"
#include "driver/uart.h"

#include "config.h"
#include "flash_param.h"
#include "mqtt_app.h"
#include "uart_app.h"

#include "flow_app.h"

os_event_t flowTaskQueue[flowTaskQueueLen];

static void ICACHE_FLASH_ATTR flowTask(os_event_t* event)
{
  switch (event->sig)
  {
    case SIG_UART:
    uartTask(event);
    break;

    case SIG_MQTT:
    MQTT_Task(event);
    break;
    
    default:
    // do nothing
    break;
  }
}

void ICACHE_FLASH_ATTR initFlow(flash_param_t* flashParam)
{
  system_os_task(flowTask, flowTaskPrio, flowTaskQueue, flowTaskQueueLen);
}
