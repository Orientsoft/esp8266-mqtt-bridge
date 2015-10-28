#ifndef __FLASH_PARAM_H__
#define __FLASH_PARAM_H__

#define FLASH_PARAM_MAGIC	9527
#define FLASH_PARAM_VERSION	2

typedef struct flash_param {
	uint32_t magic;
	uint32_t version;

  // uart params
	uint32_t baud; 
	uint8_t uartconf0; // UART_CONF0 register register : stop_bit_num 	 [5:4], bit_num [3:2] ,parity_en [1],parity	[0] 
  
  // MQTT params
  char mqttHost[64];
  uint16_t mqttPort;
  char clientId[64];
  char user[64];
  char pass[64];
  uint32_t keepalive;
  uint8_t cleanSession;

  // padding
	// char padding[2]; // set array index so that the flash area is readable as data with aligned 4-byte reads.
} flash_param_t; 

flash_param_t *flash_param_get(void);
int flash_param_set(void);

#endif /* __FLASH_PARAM_H__ */
