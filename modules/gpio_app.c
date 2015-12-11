/*
 * gpio_app.c
 *
 * Created on: Jun 29, 2015
 * Author: Di Xie
 */

#include "c_types.h"
#include "os_type.h"
#include "mem.h"
#include "osapi.h"
#include "user_interface.h"
#include "debug.h"
#include "flash_param.h"
#include <gpio.h>
#include "gpio_app.h"
#include "driver/key.h"

LOCAL struct keys_param keys;
LOCAL struct single_key_param* single_key[KEY_COUNT];

void GPIOConfig(void)
{
  gpio_init();
  
  PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2);
  // important: disable internel pullup
  PIN_PULLUP_DIS(PERIPHS_IO_MUX_GPIO2_U);

  key_app_init();
}

// button ISR
LOCAL void ICACHE_FLASH_ATTR longPressCb(void)
{
  // clear flash_param
  flash_param_t* flash_param = flash_param_get();
  flash_param->staFlag = 0;
  flash_param_set();

  // reset
  os_delay_us(10000);
  system_restart();
}

LOCAL void ICACHE_FLASH_ATTR shortPressCb(void)
{
  // TO DO : reset? do nothing for now
}

uint32_t ICACHE_FLASH_ATTR LED_Set(uint8_t state)
{
  if (state == 0)
    gpio_output_set(0, BIT2, BIT2, 0);
  else
    gpio_output_set(BIT2, 0, BIT2, 0);

  return 1;
}

void key_app_init()
{
  single_key[0] = key_init_single(KEY_IO_NUM, KEY_IO_MUX, KEY_IO_FUNC,
    longPressCb, shortPressCb);
  keys.key_num = KEY_COUNT;
  keys.single_key = single_key;
  
  key_init(&keys);
}
