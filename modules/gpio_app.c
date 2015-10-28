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
#include <gpio.h>
#include "gpio_app.h"

void GPIOConfig(void)
{
  gpio_init();
  
  PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2);
  PIN_PULLUP_DIS(PERIPHS_IO_MUX_GPIO2_U);

  // gpio_output_set(0, BIT2, BIT2, 0);
  // GPIO_OUTPUT_SET(2, 0);

  // INFO("GPIO: config\r\n");
}

uint32_t ICACHE_FLASH_ATTR LED_Set(uint8_t state)
{
  if (state == 0)
    gpio_output_set(0, BIT2, BIT2, 0);
  else
    gpio_output_set(BIT2, 0, BIT2, 0);

  return 1;
}

