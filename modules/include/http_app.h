#ifndef HTTP_APP_H
#define HTTP_APP_H

#define SERVER_TIMEOUT 28799
#define MAX_TX_BUFFER 1024
#define MAX_RX_BUFFER 1024

typedef struct _TCP_SERVER {
  struct espconn *conn;
  char* txBuffer;
  uint16_t txBufferLen;
  bool readyFlag;
  char* rxBuffer;
  uint16_t rxBufferLen;
} TCP_SERVER;

typedef struct _HTTP_REQ {
  char ssid[32];
  char pwd[64];
  char uuid[64];
  char token[64];
} HTTP_REQ;

enum HTTP_STAT {
  HTTP_IDLE = 0,
  HEADER_GET,
  HEADER_POST,
  DONE_GET,
  BODY_POST,
  DONE_POST
};

// interface
void ICACHE_FLASH_ATTR startHttpServer();
void ICACHE_FLASH_ATTR stopHttpServer();
void ICACHE_FLASH_ATTR resetHttpServer();
void ICACHE_FLASH_ATTR sendBuffer(TCP_SERVER* tcpServer, char* buffer, uint16_t len);

// helper
void ICACHE_FLASH_ATTR parseHttp(char* data, uint16_t len); 
void ICACHE_FLASH_ATTR respondHttp(bool getFlag);
char* ICACHE_FLASH_ATTR getHeader(uint16_t contentLen);
char* ICACHE_FLASH_ATTR getLine();
HTTP_REQ* ICACHE_FLASH_ATTR getHttpReq(char* line);

#endif
