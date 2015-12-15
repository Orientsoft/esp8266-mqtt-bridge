#ifndef TEA_H
#define TEA_H

#define CYCLE 32

uint32_t* ICACHE_FLASH_ATTR encrypt(uint32_t* v, uint32_t* k);
uint32_t* ICACHE_FLASH_ATTR decrypt(uint32_t* v, uint32_t* k);

#endif
