#include "ets_sys.h"
#include "os_type.h"
#include "osapi.h"
#include "mem.h"
#include "driver/uart.h"

#include "config.h"
#include "flash_param.h"
#include "mqtt_app.h"
#include "uart_app.h"

#define MAX_UART_BUFFER (MAX_MQTT_BUFFER/4)

#define uartTaskPrio 0
#define uartTaskQueueLen 64

os_event_t uartTaskQueue[uartTaskQueueLen];
static uint8 uart_buffer[MAX_UART_BUFFER];

extern flash_param_t* flashParam;
extern UartDevice UartDev;

void ICACHE_FLASH_ATTR uartTask(os_event_t* events)
{
  while (READ_PERI_REG(UART_STATUS(UART0)) &
    (UART_RXFIFO_CNT << UART_RXFIFO_CNT_S))
  {
    WRITE_PERI_REG(0X60000914, 0x73);
    uint16_t len = 0;
    while ((READ_PERI_REG(UART_STATUS(UART0)) &
      (UART_RXFIFO_CNT << UART_RXFIFO_CNT_S)) &&
      (len < MAX_UART_BUFFER))
      uart_buffer[len++] = READ_PERI_REG(UART_FIFO(UART0)) & 0xFF;

    #ifdef CONFIG_DYNAMIC
    if (len >=5 &&
      uart_buffer[0] == '+' &&
      uart_buffer[1] == '+' &&
      uart_buffer[2] == '+' &&
      uart_buffer[3] =='A' &&
      uart_buffer[4] == 'T')
    {
      config_parse(uart_buffer, len);
    }
    #endif

    // pub uart_buffer
    char* pubTopic = (char*)os_zalloc(os_strlen(flashParam->user) + 13);

    pubTopic[0] = 0;
    os_strcat(pubTopic, "/devices/");
    os_strcat(pubTopic, flashParam->user);
    os_strcat(pubTopic, "/in");
    MQTT_Publish(mqttClient, pubTopic, uart_buffer, len, 0, 0);

    os_free(pubTopic);
  }

  if (UART_RXFIFO_FULL_INT_ST == (READ_PERI_REG(UART_INT_ST(UART0)) &
    UART_RXFIFO_FULL_INT_ST))
    WRITE_PERI_REG(UART_INT_CLR(UART0), UART_RXFIFO_FULL_INT_CLR);
  else if (UART_RXFIFO_TOUT_INT_ST == (READ_PERI_REG(UART_INT_ST(UART0)) &
    UART_RXFIFO_TOUT_INT_ST))
    WRITE_PERI_REG(UART_INT_CLR(UART0), UART_RXFIFO_TOUT_INT_CLR);
  
  //ETS_UART_INTR_ENABLE();
  uart_rx_intr_enable(UART0);
}

void ICACHE_FLASH_ATTR initUart(flash_param_t* flashParam)
{
  // config uart
  #ifdef CONFIG_DYNAMIC
  UartDev.data_bits = GETUART_DATABITS(flashParam->uartconf0);
  UartDev.parity = GETUART_PARITYMODE(flashParam->uartconf0);
  UartDev.stop_bits = GETUART_STOPBITS(flashParam->uartconf0);
	uart_init(flashParam->baud, BIT_RATE_115200);
  #else
  UartDev.data_bits = EIGHT_BITS;
  UartDev.parity = NONE_BITS;
  UartDev.stop_bits = ONE_STOP_BIT;
  uart_init(BIT_RATE_115200, BIT_RATE_115200);
  #endif

  UART_SetPrintPort(1);
  // uart0_sendStr("initUart\r\n");

  // system_os_task(uartTask, uartTaskPrio, uartTaskQueue, uartTaskQueueLen);
}
