#include "user_interface.h"
#include "osapi.h"
#include "mem.h"
#include "flash_param.h"
#include "tea.h"
#include "crypt_app.h"

uint32_t goldenKey[2] = { 0x9527, 0x2795 };
uint32_t* globalKey;

uint32_t* ICACHE_FLASH_ATTR keyGen(uint32_t* keySrc)
{
  uart0_sendStr("Entering keyGen\r\n");

  uint32_t* key = (uint32_t*)os_zalloc(4 * sizeof(uint32_t));
  os_memset(key, 0, 4 * sizeof(uint32_t));
  
  uint32_t* result = encrypt(keySrc, (uint32_t*)goldenKey);
  os_memcpy(key, result, 2 * sizeof(uint32_t));
  os_free(result);

  result = encrypt(keySrc + 2, (uint32_t*)goldenKey);
  os_memcpy(key + 2, result, 2 * sizeof(uint32_t));
  os_free(result);

  uart0_sendStr("Leaving keyGen\r\n");

  return key;
}

uint32_t* ICACHE_FLASH_ATTR bufCrypt(uint32_t* str, uint32_t count, uint32_t* key)
{
  uart0_sendStr("Entering bufCrypt\r\n");

  if (count % 2 != 0)
    return NULL;

  uint32_t* result = (uint32_t*)os_zalloc(count * sizeof(uint32_t));
  os_memset(result, 0, count * sizeof(uint32_t));

  uint32_t* tmp;
  uint32_t i;
  for (i = 0; i < count / 2; i++)
  {
    char* iStr = (char*)os_zalloc(16);
    os_sprintf(iStr, "i: %d\r\n", i);
    uart0_sendStr(iStr);
    os_free(iStr);
    
    tmp = encrypt(str + i * 2, key);
    os_memcpy(result + i * 2, tmp, 2 * sizeof(uint32_t));
    os_free(tmp);
  }

  uart0_sendStr("Leaving bufCrypt\r\n");

  return result;
}

uint32_t* ICACHE_FLASH_ATTR bufDecrypt(uint32_t* buf, uint32_t count, uint32_t* key)
{
  uart0_sendStr("Entering bufDecrypt\r\n");

  if (count % 2 != 0)
    return NULL;

  uint32_t* result = (uint32_t*)os_zalloc(count * sizeof(uint32_t));
  os_memset(result, 0, count * sizeof(uint32_t));

  uint32_t* tmp;
  uint32_t i;
  for (i = 0; i < count / 2; i++)
  {
    char* iStr = (char*)os_zalloc(16);
    os_sprintf(iStr, "i: %d\r\n", i);
    uart0_sendStr(iStr);
    os_free(iStr);

    tmp = decrypt(buf + i * 2, key);
    os_memcpy(result + i * 2, tmp, 2 * sizeof(uint32_t));
    os_free(tmp);
  }

  uart0_sendStr("Leaving bufDecrypt\r\n");

  return result;
}

void flashCrypt(flash_param_t* flashParam)
{
  uart0_sendStr("Entering flashCrypt\r\n");

  uint32_t* tmp = bufCrypt((uint32_t*)flashParam->mqttHost, 16, globalKey);
  uart0_sendStr(tmp);
  uint32_t* tmp1 = bufDecrypt(tmp, 16, globalKey);
  uart0_sendStr(tmp1);

  // yes it's 64 bytes
  os_memcpy(flashParam->mqttHost, tmp, 64);
  os_free(tmp);

  uart0_sendStr("Leaving flashCrypt\r\n");
}

void flashDecrypt(flash_param_t* flashParam)
{
  uart0_sendStr("Leaving flashDecrypt\r\n");

  uart0_sendStr(flashParam->mqttHost);
  uint32_t* tmp = bufDecrypt((uint32_t*)flashParam->mqttHost, 16, globalKey);
  uart0_sendStr(tmp);
  // yes it's 64 bytes
  os_memcpy(flashParam->mqttHost, tmp, 64);
  os_free(tmp); 

  uart0_sendStr("Leaving flashDecrypt\r\n");
}

