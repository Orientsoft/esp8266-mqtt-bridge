/*
 * gpio_app.h
 *
 * Created on: Jun 29, 2015
 * Author: Di Xie
 */

#ifndef MODULES_GPIO_APP_H
#define MODULES_GPIO_APP_H

#define KEY_COUNT 1

#define KEY_IO_MUX PERIPHS_IO_MUX_MTCK_U
#define KEY_IO_NUM 13
#define KEY_IO_FUNC FUNC_GPIO13

void GPIOConfig(void);
uint32_t ICACHE_FLASH_ATTR LED_Set(uint8_t state);

void key_app_init(void);

#endif
