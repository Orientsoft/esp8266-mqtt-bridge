#ifndef CRYPT_APP_H
#define CRYPT_APP_H

extern uint32_t* globalKey;

void flashCrypt(flash_param_t* flashParam);
void flashDecrypt(flash_param_t* flashParam);

uint32_t* ICACHE_FLASH_ATTR keyGen(uint32_t* keySrc);

uint32_t* ICACHE_FLASH_ATTR bufCrypt(uint32_t* buf, uint32_t count, uint32_t* key);
uint32_t* ICACHE_FLASH_ATTR bufDecrypt(uint32_t* buf, uint32_t count, uint32_t* key);


#endif
