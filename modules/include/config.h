#ifndef __CONFIG_H__
#define __CONFIG_H__

//#define CONFIG_STATIC
#ifdef CONFIG_STATIC

#define STA_SSID	"STA_SSID"
#define STA_PASSWORD	"STA_PWD"
#define AP_SSID		"AP_SSID"
#define AP_PASSWORD	"AP_PWD"

void config_execute(void);

#endif

// this works for now
#define CONFIG_DYNAMIC
#ifdef CONFIG_DYNAMIC

typedef struct config_commands {
	char *command;
	void(*function)(uint8_t argc, char *argv[]);
} config_commands_t;

void config_parse(const char *buf, int len);

#endif

#define CONFIG_GPIO

#ifdef CONFIG_GPIO
#include <gpio.h>

void config_gpio(void);

#endif

#endif /* __CONFIG_H__ */
