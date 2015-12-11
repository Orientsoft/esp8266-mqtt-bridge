#include "ets_sys.h"
#include "osapi.h"
#include "user_interface.h"
#include "mem.h"
#include "gpio_app.h"
#include "driver/uart.h"
#include "flash_param.h"
#include "mqtt_app.h"
#include "uart_app.h"
#include "http_app.h"

flash_param_t* flashParam;

void user_rf_pre_init(void)
{
}

void wifiCallback(uint8_t wifiStatus)
{
  if (wifiStatus == STATION_GOT_IP)
  {
    initMqtt(flashParam);
  }
}

void initCb(void)
{
  WIFI_Connect(flashParam, wifiCallback);
}

void ICACHE_FLASH_ATTR
user_init(void)
{
  flash_param_init();
  flashParam = (flash_param_t*)flash_param_get();
  
  // start web server
  if (flashParam->staFlag == 0)
  {
    // wifi ap
    wifi_set_opmode(SOFTAP_MODE);
    struct softap_config apConfig;
    os_bzero(&apConfig, sizeof(struct softap_config));
    wifi_softap_get_config_default(&apConfig);

    os_memcpy(apConfig.ssid, "ESP_CONF", 9);
    apConfig.ssid_len = os_strlen(apConfig.ssid);
    os_memcpy(apConfig.password, "orientsoft", 11);
    apConfig.authmode = AUTH_WPA2_PSK;
    
    wifi_softap_set_config(&apConfig);
    
    initUart(flashParam);

    // start http
    startHttpServer();
  }
  else
  {
    initUart(flashParam);
    uart0_sendStr("\r\nesp-mqtt-bridge\r\n");
    char buf[128];
    os_sprintf(buf, "%d\r\n", flashParam->version);
    uart0_sendStr("FLASH PARAM VER. ");
    uart0_sendStr(buf);

    char* idStr = (char*)os_zalloc(32);
    os_sprintf(idStr, "\r\nCHIP_ID: %d\r\n", system_get_chip_id());
    uart0_sendStr(idStr);
    os_free(idStr);

    GPIOConfig();
    initFlow();

    system_init_done_cb(initCb);
  }
}
