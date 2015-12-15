#include "user_interface.h"
#include "osapi.h"
#include "mem.h"
#include "tea.h"

uint32_t delta = 0x9e3779b9;
uint32_t decryptSum = 0xc6ef3720;

uint32_t* ICACHE_FLASH_ATTR encrypt(uint32_t* v, uint32_t* k)
{
  // setup
  uint32_t y = v[0];
  uint32_t z = v[1];

  uint32_t sum = 0;
  uint32_t i;

  // cache key
  uint32_t a = k[0];
  uint32_t b = k[1];
  uint32_t c = k[2];
  uint32_t d = k[3];

  uint32_t* result = (uint32_t*)os_zalloc(2);
  os_memset(result, 0, 2);

  // cycle
  for (i = 0; i < CYCLE; i++)
  {
    sum += delta;
    y += ((z << 4) + a) ^ (z + sum) ^ ((z >> 5) + b);
    z += ((y << 4) + c) ^ (y + sum) ^ ((y >> 5) + d);
  }

  result[0] = y;
  result[1] = z;

  return result;
}

uint32_t* ICACHE_FLASH_ATTR decrypt(uint32_t* v, uint32_t* k)
{
  // setup
  uint32_t y = v[0];
  uint32_t z = v[1];

  uint32_t sum = decryptSum;

  // cache key
  uint32_t a = k[0];
  uint32_t b = k[1];
  uint32_t c = k[2];
  uint32_t d = k[3];

  uint32_t* result = (uint32_t*)os_zalloc(2);
  os_memset(result, 0, 2);

  // cycle
  uint32_t i;
  for (i = 0; i < 32; i++)
  {
    z -= ((y << 4) + c) ^ (y + sum) ^ ((y >> 5) + d);
    y -= ((z << 4) + a) ^ (z + sum) ^ ((z >> 5) + b);
    sum -= delta;
  }

  result[0] = y;
  result[1] = z;
}

