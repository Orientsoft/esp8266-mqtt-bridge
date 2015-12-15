#include "user_interface.h"
#include "osapi.h"
#include "mem.h"
#include "flash_param.h"
#include "tea.h"
#include "crypt_app.h"

uint32_t goldenKey[2] = { 0x9527, 0x2795 };

uint32_t* ICACHE_FLASH_ATTR keyGen(uint32_t* keySrc)
{
  uint32_t* key = (uint32_t*)os_zalloc(4);
  os_memset(key, 0, 4);

  uint32_t* result = encrypt(keySrc, (uint32_t*)goldenKey);
  os_memcpy(key, result, 2);
  os_free(result);

  result = encrypt(keySrc + 2, (uint32_t*)goldenKey);
  os_memcpy(key + 2, result, 2);
  os_free(result);

  return key;
}

uint32_t* ICACHE_FLASH_ATTR bufCrypt(uint32_t* str, uint32_t count, uint32_t* key)
{
  if (count % 2 != 0)
    return NULL;

  uint32_t* result = (uint32_t*)os_zalloc(count * 4);
  os_memset(result, 0, count * 4);

  uint32_t* tmp;
  uint32_t i;
  for (i = 0; i < count / 2; i++)
  {
    tmp = encrypt(str + i * 2, key);
    os_memcpy(result + i * 2, tmp, 2);
    os_free(tmp);
  }

  return result;
}

uint32_t* ICACHE_FLASH_ATTR bufDecrypt(uint32_t* buf, uint32_t count, uint32_t* key)
{
  if (count % 2 != 0)
    return NULL;

  uint32_t* result = (uint32_t*)os_zalloc(count * 4);
  os_memset(result, 0, count * 4);

  uint32_t* tmp;
  uint32_t i;
  for (i = 0; i < count / 2; i++)
  {
    tmp = decrypt(buf + i * 2, key);
    os_memcpy(result + i * 2, tmp, 2);
    os_free(tmp);
  }

  return result;
}

void flashCrypt(flash_param_t* flashParam)
{
  uint32_t chipId = system_get_chip_id();
  uint32_t keySrc[2] = { chipId, 0x9527 };
  uint32_t* key = keyGen((uint32_t*)keySrc);

  uint32_t* tmp = bufCrypt((uint32_t*)flashParam->mqttHost, 16, key);
  os_memcpy(flashParam->mqttHost, tmp, 64);
  os_free(tmp);
}

void flashDecrypt(flash_param_t* flashParam)
{
  uint32_t chipId = system_get_chip_id();
  uint32_t keySrc[2] = { chipId, 0x9527 };
  uint32_t* key = keyGen((uint32_t*)keySrc);

  uint32_t* tmp = bufDecrypt((uint32_t*)flashParam->mqttHost, 16, key);
  os_memcpy(flashParam->mqttHost, tmp, 64);
  os_free(tmp); 
}

