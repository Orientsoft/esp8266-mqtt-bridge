#include "ets_sys.h"
#include "osapi.h"
#include "user_interface.h"
#include "mem.h"
#include "gpio_app.h"
#include "driver/uart.h"
#include "flash_param.h"
#include "mqtt_app.h"
#include "uart_app.h"

flash_param_t* flashParam;

void user_rf_pre_init(void)
{
}

void wifiCallback(uint8_t wifiStatus)
{
  // uart0_sendStr("wifiCallback\r\n");

  if (wifiStatus == STATION_GOT_IP)
  {
    // uart0_sendStr("STATION_GOT_IP\r\n");
    initMqtt(flashParam);
  }
}

void initCb(void)
{
  // uart0_sendStr("initCB\r\n");
  WIFI_Connect(flashParam, wifiCallback);
}

void ICACHE_FLASH_ATTR
user_init(void)
{
  flash_param_init();
  flashParam = (flash_param_t*)flash_param_get();

  initUart(flashParam);
  // uart0_sendStr("\r\nesp-mqtt-bridge\r\n");
  // char buf[128];
  // os_sprintf(buf, "%d\r\n", flashParam->version);
  // uart0_sendStr("FLASH PARAM VER. ");
  // uart0_sendStr(buf);

  GPIOConfig();
  initFlow();

	system_init_done_cb(initCb);
}
