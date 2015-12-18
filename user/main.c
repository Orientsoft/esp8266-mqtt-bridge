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
#include "tea.h"
#include "crypt_app.h"

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
    initUart(flashParam);

    // wifi ap
    wifi_set_opmode(SOFTAP_MODE);
    struct softap_config apConfig;
    os_bzero(&apConfig, sizeof(struct softap_config));
    wifi_softap_get_config_default(&apConfig);
    
    os_memcpy(apConfig.ssid, "ESP_CONF", 9);
    apConfig.ssid_len = os_strlen(apConfig.ssid);
    os_memcpy(apConfig.password, "orientsoft", 11);
    apConfig.authmode = AUTH_WPA2_PSK;
    
    ETS_UART_INTR_DISABLE();
    wifi_softap_set_config(&apConfig);
    ETS_UART_INTR_ENABLE();
    
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

    uint32_t chipId = system_get_chip_id();
    char* idStr = (char*)os_zalloc(32);
    os_sprintf(idStr, "CHIP_ID: %X\r\n", chipId);
    uart0_sendStr(idStr);
    os_free(idStr);

    uint32_t keySrc[2] = { chipId, 0x9527 };
    uint32_t* key = keyGen((uint32_t*)keySrc);
    globalKey = key;
    
    char* cryptStr = (char*)os_zalloc(128);
    memset(cryptStr, 0, 128);
    os_sprintf(cryptStr, "key: %X %X %X %X\r\n", key[0], key[1], key[2], key[3]);
    uart0_sendStr(cryptStr);

    char* src = "Hello, ESP... This is TEA tests.";
    uint32_t* srcInt = (uint32_t*)os_zalloc(8 * sizeof(uint32_t));
    os_memcpy(srcInt, src, 32);
    uint32_t* dest = bufCrypt((uint32_t*)srcInt, 8, key);
    memset(cryptStr, 0, 128);
    os_sprintf(cryptStr, "src: %s\r\n", src);
    uart0_sendStr(cryptStr);

    memset(cryptStr, 0, 128);
    os_sprintf(cryptStr, "dest: %s\r\n", dest);
    uart0_sendStr(cryptStr);

    uint32_t* newSrc = bufDecrypt(dest, 8, key);
    memset(cryptStr, 0, 128);
    os_sprintf(cryptStr, "decrypt: %s\r\n", newSrc);
    uart0_sendStr(cryptStr);

    os_free(cryptStr);

    uart0_sendStr("MQTT HOST: ");
    uart0_sendStr(flashParam->mqttHost);
    uart0_sendStr("\r\n");

    flashCrypt(flashParam);
    uart0_sendStr("\r\nMQTT HOST ENCRYPT: ");
    uart0_sendStr(flashParam->mqttHost);
    uart0_sendStr("\r\n");

    flashDecrypt(flashParam);
    uart0_sendStr("\r\nMQTT HOST DECRYPT: ");
    uart0_sendStr(flashParam->mqttHost);
    uart0_sendStr("\r\n");

    GPIOConfig();
    initFlow();

    system_init_done_cb(initCb);
  }
}
