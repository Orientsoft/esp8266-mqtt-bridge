/*
 * wifi.c
 *
 *  Created on: Dec 30, 2014
 *      Author: Minh
 */
#include "user_interface.h"
#include "osapi.h"
#include "espconn.h"
#include "os_type.h"
#include "mem.h"

#include "user_config.h"
#include "debug.h"
#include "flash_param.h"
#include "wifi_app.h"

static ETSTimer WiFiLinker;
WifiCallback wifiCb = (WifiCallback)NULL;
static uint8_t wifiStatus = STATION_IDLE, lastWifiStatus = STATION_IDLE;

static void ICACHE_FLASH_ATTR wifi_check_ip(void *arg)
{
	struct ip_info ipConfig;

	os_timer_disarm(&WiFiLinker);
	wifi_get_ip_info(STATION_IF, &ipConfig);
	wifiStatus = wifi_station_get_connect_status();
	if (wifiStatus == STATION_GOT_IP && ipConfig.ip.addr != 0)
	{
    // watcher
		os_timer_setfn(&WiFiLinker, (os_timer_func_t *)wifi_check_ip, NULL);
		os_timer_arm(&WiFiLinker, 2000, 0);
	}
	else
	{
    // if unsuccessful, retry
		if(wifi_station_get_connect_status() == STATION_WRONG_PASSWORD)
		{
			// uart0_sendStr("STATION_WRONG_PASSWORD\r\n");
			wifi_station_connect();
		}
		else if(wifi_station_get_connect_status() == STATION_NO_AP_FOUND)
		{
			// uart0_sendStr("STATION_NO_AP_FOUND\r\n");
			wifi_station_connect();
		}
		else if(wifi_station_get_connect_status() == STATION_CONNECT_FAIL)
		{
			// uart0_sendStr("STATION_CONNECT_FAIL\r\n");
			wifi_station_connect();
		}
		else
		{
			// uart0_sendStr("STATION_IDLE\r\n");
		}

		os_timer_setfn(&WiFiLinker, (os_timer_func_t *)wifi_check_ip, NULL);
		os_timer_arm(&WiFiLinker, 500, 0);
	}

	if(wifiStatus != lastWifiStatus)
  {
    // notify when status changes
		lastWifiStatus = wifiStatus;
    wifiCb(wifiStatus);
	}
}

uint32_t ICACHE_FLASH_ATTR
WIFI_Connect(flash_param_t* flashParam, WifiCallback callback)
{
	struct station_config stationConf;
  wifi_station_get_config_default(&stationConf);

  uart0_sendStr("WIFI_Connect\r\n");
  uart0_sendStr(stationConf.ssid);
  uart0_sendStr("\r\n");
  uart0_sendStr(stationConf.password);
  uart0_sendStr("\r\n");

	wifi_station_set_auto_connect(FALSE);
	wifi_set_opmode(STATION_MODE);

	wifi_station_set_config(&stationConf);

  wifiCb = callback;

	os_timer_disarm(&WiFiLinker);
	os_timer_setfn(&WiFiLinker, (os_timer_func_t *)wifi_check_ip, NULL);
	os_timer_arm(&WiFiLinker, 1000, 0);

	wifi_station_set_auto_connect(TRUE);
	wifi_station_connect();
	return 0;
}

