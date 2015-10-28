#ifndef CONFIG_PARSE_TEST_UNIT

// this is the normal build target ESP include set
// #include "espmissingincludes.h"
#include "c_types.h"
#include "user_interface.h"
#include "mem.h"
#include "osapi.h"
#include "driver/uart.h"

#include "flash_param.h"
#include "mqtt_app.h"

#else

// test unit target for config_parse
// gcc -g -o config_test config.c -D CONFIG_PARSE_TEST_UNIT
// ./config_test < config_test.cmd
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

struct station_config {
	uint8_t ssid[32];
	uint8_t password[64];
	uint8_t bssid_set;
	uint8_t bssid[6];
};

#define os_sprintf	sprintf
#define os_malloc	malloc
#define os_strncpy	strncpy
#define os_strncmp	strncmp
#define os_free		free
#define os_bzero	bzero
#define os_memcpy	memcpy
#define os_memset	memset

#define AUTH_OPEN 0
#define AUTH_WPA_PSK 2
#define AUTH_WPA2_PSK 3

#define wifi_get_opmode() (printf("wifi_get_opmode()\n") ? 2 : 2)
#define wifi_set_opmode(mode) printf("wifi_set_opmode(%d)\n", mode)
#define wifi_station_disconnect() printf("wifi_station_disconnect()\n")
#define wifi_station_get_config(conf) { strncpy((conf)->ssid, "dummystassid", 32); strncpy((conf)->password, "dummystapassword", 64); }
#define wifi_station_set_config(conf) printf("wifi_station_set_config(%s, %s)\n", (conf)->ssid, (conf)->password)
#define wifi_station_connect() printf("wifi_station_connect()\n");
#define wifi_get_macaddr(if, result) printf("wifi_get_mac_addr(SOFTAP_IF, macaddr)\n")
#define wifi_softap_get_config(conf) { strncpy((conf)->ssid, "dummyapssid", 32); strncpy((conf)->password, "dummyappassword", 64); (conf)->authmode=AUTH_WPA_PSK; (conf)->channel=3; }
#define wifi_softap_set_config(conf) printf("wifi_softap_set_config(%s, %s, %d, %d)\n", (conf)->ssid, (conf)->password, (conf)->authmode, (conf)->channel)
// mqtt config init
#define mqtt_get_config(conf) { strncpy((conf)->host, "z.borgnix.com", 64); (conf)->port = 1883; strncpy((conf)->clientId, "12345", 64); strncpy((conf)->user, "uuid", 64); strncpy((conf)->pass, "token", 64); (conf)->keepalive = 120; (conf)->cleanSession = 1; }
#define system_restart() printf("system_restart()\n");
#define ETS_UART_INTR_DISABLE()	printf("ETS_UART_INTR_DISABLE()\n");
#define ETS_UART_INTR_ENABLE()	printf("ETS_UART_INTR_ENABLE()\n");

#endif

#include "config.h"

#ifdef CONFIG_GPIO
void config_gpio(void) {
	// Initialize the GPIO subsystem.
	gpio_init();
	//Set GPIO2 to output mode
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2);
	//Set GPIO2 high
	gpio_output_set(BIT2, 0, BIT2, 0);
}
#endif

#ifdef CONFIG_STATIC

void config_execute(void) {
	uint8_t mode;
	struct station_config sta_conf;
	uint8_t macaddr[6] = { 0, 0, 0, 0, 0, 0 };

	mode = wifi_get_opmode();
	if (mode != STATIONAP_MODE) {
		wifi_set_opmode(STATIONAP_MODE);
		os_delay_us(10000);
		system_restart();
	}

	// connect to our station
	os_bzero(&sta_conf, sizeof(struct station_config));
	wifi_station_get_config(&sta_conf);
	os_strncpy(sta_conf.ssid, STA_SSID, sizeof(sta_conf.ssid));
	os_strncpy(sta_conf.password, STA_PASSWORD, sizeof(sta_conf.password));
	wifi_station_disconnect();
	ETS_UART_INTR_DISABLE();
	wifi_station_set_config(&sta_conf);
	ETS_UART_INTR_ENABLE();
	wifi_station_connect();
}

#endif

#ifdef CONFIG_DYNAMIC

#define MSG_OK "OK\r\n"
#define MSG_ERROR "ERROR\r\n"
#define MSG_INVALID_CMD "UNKNOWN COMMAND\r\n"

#define MAX_ARGS 12
#define MSG_BUF_LEN 128

#ifdef CONFIG_PARSE_TEST_UNIT
#endif

bool doflash = true;

char *my_strdup(char *str) {
	size_t len;
	char *copy;

	len = strlen(str) + 1;
	if (!(copy = (char*)os_malloc((u_int)len)))
		return (NULL);
	os_memcpy(copy, str, len);
	return (copy);
}

char **config_parse_args(char *buf, uint8_t *argc) {
	const char delim[] = " \t";
	char *save, *tok;
	char **argv = (char **)os_malloc(sizeof(char *) * MAX_ARGS);	// note fixed length
	os_memset(argv, 0, sizeof(char *) * MAX_ARGS);

	*argc = 0;
	for (; *buf == ' ' || *buf == '\t'; ++buf); // absorb leading spaces
	for (tok = strtok_r(buf, delim, &save); tok; tok = strtok_r(NULL, delim, &save)) {
		argv[*argc] = my_strdup(tok);
		(*argc)++;
		if (*argc == MAX_ARGS) {
			break;
		}
	}
	return argv;
}

void config_parse_args_free(uint8_t argc, char *argv[]) {
	uint8_t i;
	for (i = 0; i <= argc; ++i) {
		if (argv[i])
			os_free(argv[i]);
	}
	os_free(argv);
}

void config_cmd_reset(uint8_t argc, char *argv[]) {
	system_restart();
}


#ifdef CONFIG_GPIO
void config_cmd_gpio2(uint8_t argc, char *argv[]) {
	if (argc == 0)
  {
	  // os_printf("ARGC = 0\r\n");
    ;
  }
  else {
    os_printf("LED = %s\r\n", argv[1]);
    uint8_t state = atoi(argv[1]);
    LED_Set(state);
  }
}
#endif

void config_cmd_baud(uint8_t argc, char *argv[]) {
	flash_param_t *flash_param = flash_param_get();
	UartBitsNum4Char data_bits = GETUART_DATABITS(flash_param->uartconf0);
	UartParityMode parity = GETUART_PARITYMODE(flash_param->uartconf0);
	UartStopBitsNum stop_bits = GETUART_STOPBITS(flash_param->uartconf0);
	const char *stopbits[4] = { "?", "1", "1.5", "2" };
	const char *paritymodes[4] = { "E", "O", "N", "?" };
	if (argc == 0)
  {
		// espbuffsentprintf(conn, "BAUD=%d %d %s %s\r\n"MSG_OK, flash_param->baud,data_bits + 5, paritymodes[parity], stopbits[stop_bits]);
  }
	else {
		uint32_t baud = atoi(argv[1]);
		if ((baud > (UART_CLK_FREQ / 16)) || baud == 0) {
			// TO DO : error
      return;
		}
		if (argc > 1) {
			data_bits = atoi(argv[2]);
			if ((data_bits < 5) || (data_bits > 8)) {
				// TO DO : error
        return;
			}
			data_bits -= 5;
		}
		if (argc > 2) {
			if (strcmp(argv[3], "N") == 0)
				parity = NONE_BITS;
			else if (strcmp(argv[3], "O") == 0)
				parity = ODD_BITS;
			else if (strcmp(argv[3], "E") == 0)
				parity = EVEN_BITS;
			else {
        // TO DO : error
        ;
				return;
			}
		}
		if (argc > 3) {
			if (strcmp(argv[4], "1")==0)
				stop_bits = ONE_STOP_BIT;
			else if (strcmp(argv[4], "2")==0)
				stop_bits = TWO_STOP_BIT;
			else if (strcmp(argv[4], "1.5") == 0)
				stop_bits = ONE_HALF_STOP_BIT;
			else {
				// TO DO : error
        ;
        return;
			}
		}
		// pump and dump fifo
		while (TRUE) {
			uint32_t fifo_cnt = READ_PERI_REG(UART_STATUS(0)) & (UART_TXFIFO_CNT << UART_TXFIFO_CNT_S);
			if ((fifo_cnt >> UART_TXFIFO_CNT_S & UART_TXFIFO_CNT) == 0) {
				break;
			}
		}
		os_delay_us(10000);
		uart_div_modify(UART0, UART_CLK_FREQ / baud);
		flash_param->baud = baud;
		flash_param->uartconf0 = CALC_UARTMODE(data_bits, parity, stop_bits);
		WRITE_PERI_REG(UART_CONF0(UART0), flash_param->uartconf0);
		if (doflash) {
			if (flash_param_set())
			  // TO DO : OK
        ;
      else
        // TO DO : error
        ;
		}
		else
      // TO DO : OK
      ;
	}
}

void config_cmd_flash(uint8_t argc, char *argv[]) {
	bool err = false;
	if (argc == 0)
		// espbuffsentprintf(conn, "FLASH=%d\r\n", doflash);
    ;
	else if (argc != 1)
		err=true;
	else {
		if (strcmp(argv[1], "1") == 0)
			doflash = true;
		else if (strcmp(argv[1], "0") == 0)
			doflash = false;
		else
			err=true;
	}
	if (err)
    // TO DO : error
    ;
	else
    // TO DO : OK
    ;
}

void config_cmd_ping(uint8_t argc, char* argv[]) {
  if (mqttConnectionFlag)
    MQTT_Publish(mqttClient, pubTopic, "ESP PING_RSP\r\n", 14, 0, 0);

  uart0_sendStr("ESP PING_RSP\r\n");
}

// spaces are not supported in the ssid or password
void config_cmd_sta(uint8_t argc, char *argv[]) {
	char *ssid = argv[1], *password = argv[2];
	struct station_config sta_conf;

	os_bzero(&sta_conf, sizeof(struct station_config));
	wifi_station_get_config(&sta_conf);

	if (argc == 0)
		// espbuffsentprintf(conn, "SSID=%s PASSWORD=%s\r\n"MSG_OK, sta_conf.ssid, sta_conf.password);
    ;
	 else if (argc != 2)
		// espbuffsentstring(conn, MSG_ERROR);
    ;
	else {
		os_strncpy(sta_conf.ssid, ssid, sizeof(sta_conf.ssid));
		os_strncpy(sta_conf.password, password, sizeof(sta_conf.password));
		// espbuffsentstring(conn, MSG_OK);
		wifi_station_disconnect();
		ETS_UART_INTR_DISABLE();
		wifi_station_set_config(&sta_conf);
		ETS_UART_INTR_ENABLE();
		wifi_station_connect();
	}
}

// MQTT config
void config_cmd_mqtt(uint8_t argc, char * argv[])
{
  flash_param_t* flash_param = flash_param_get();
  if (argc == 0)
  {
    // espbuffersentprintf(conn, "MQTT_HOST=%s MQTT_PORT=%d MQTT_CLIENT_ID=%s MQTT_USER=%s MQTT_PASS=%s KEEPALIVE=%d CLEAN_SESSION=%d", argv[1], atoi(argv[2]), argv[3], argv[4], argv[5], atoi(argv[6]), atoi(argv[7]);
  }
  else
  {
    if (argc == 2)
    {
      os_strncpy(flash_param->mqttHost, argv[1], sizeof(flash_param->mqttHost));
      flash_param->mqttPort = atoi(argv[2]);
    }
    else if (argc == 5)
    {
      os_strncpy(flash_param->mqttHost, argv[1], sizeof(flash_param->mqttHost));
      flash_param->mqttPort = atoi(argv[2]);
      os_strncpy(flash_param->clientId, argv[3], sizeof(flash_param->clientId));
      os_strncpy(flash_param->user, argv[4], sizeof(flash_param->user));
      os_strncpy(flash_param->pass, argv[5], sizeof(flash_param->pass));
    }
    else if (argc == 7)
    {
      os_strncpy(flash_param->mqttHost, argv[1], sizeof(flash_param->mqttHost));
      flash_param->mqttPort = atoi(argv[2]);
      os_strncpy(flash_param->clientId, argv[3], sizeof(flash_param->clientId));
      os_strncpy(flash_param->user, argv[4], sizeof(flash_param->user));
      os_strncpy(flash_param->pass, argv[5], sizeof(flash_param->pass));
      flash_param->keepalive = atoi(argv[6]);
      flash_param->cleanSession = atoi(argv[7]);
    }
    else
    {
      // error
    }

    // save and reset
    flash_param_set();
    os_delay_us(10000);
    system_restart();
  }
}

const config_commands_t config_commands[] = {
		{ "RESET", &config_cmd_reset },
		{ "BAUD", &config_cmd_baud },
		{ "STA", &config_cmd_sta },
		{ "FLASH", &config_cmd_flash },
    // add user control of MQTT
    { "MQTT", &config_cmd_mqtt },
    { "GPIO", &config_cmd_gpio2 },
    { "PING", &config_cmd_ping },
		{ NULL, NULL }
	};

void config_parse(const char *buf, int len) {
  char *lbuf = (char *)os_malloc(len + 1), **argv;
	uint8_t i, argc;
	// we need a '\0' end of the string
	os_memcpy(lbuf, buf, len);
	lbuf[len] = '\0';

  os_printf("AT: %s\r\n", lbuf);

	// remove any CR / LF
	for (i = 0; i < len; ++i)
		if (lbuf[i] == '\n' || lbuf[i] == '\r')
			lbuf[i] = '\0';

	// verify the command prefix
	if (os_strncmp(lbuf, "+++AT", 5) != 0) {
		return;
	}
	// parse out buffer into arguments
	argv = config_parse_args(&lbuf[5], &argc);

	if (argc == 0) {
		// espbuffsentstring(conn, MSG_OK);
    ;
	} else {
		argc--;	// to mimic C main() argc argv
		for (i = 0; config_commands[i].command; ++i) {
			if (os_strncmp(argv[0], config_commands[i].command, strlen(argv[0])) == 0) {
				config_commands[i].function(argc, argv);
				break;
			}
		}
		if (!config_commands[i].command)
			// espbuffsentstring(conn, MSG_INVALID_CMD);
      ;
	}
	config_parse_args_free(argc, argv);
	os_free(lbuf);
}

#ifdef CONFIG_PARSE_TEST_UNIT
const int max_line = 255;
int main(int argc, char *argv[]) {
	char line[max_line];

	// read lines and feed them to config_parse
	while (fgets(line, max_line, stdin) != NULL) {
		uint8_t len = strlen(line);
		config_parse(NULL, line, len);
	}
}
#endif

#endif
