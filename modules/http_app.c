#include <ip_addr.h>
#include <c_types.h>
#include <mem.h>
#include <osapi.h>
#include <espconn.h>
#include <user_interface.h>

#include "http_app.h"
#include "flash_param.h"

TCP_SERVER* tcpServer;
struct espconn* tcpConn;
esp_tcp* espTcp;

enum HTTP_STAT httpStat;
HTTP_REQ* httpReq;

char* httpBuffer;

int8_t ICACHE_FLASH_ATTR sendTxBuffer(TCP_SERVER* tcpServer)
{
  int8_t result = ESPCONN_OK;
  if (tcpServer->txBufferLen != 0)
  {
    tcpServer->readyFlag = false;
    result = espconn_sent(tcpServer->conn, (uint8_t*)tcpServer->txBuffer, tcpServer->txBufferLen);
    tcpServer->txBufferLen = 0;

    return result;
  }
}

void ICACHE_FLASH_ATTR tcpSentCb(void* arg)
{
  tcpServer->readyFlag = true;

  // send if there's more data
  sendTxBuffer(tcpServer);
}

void ICACHE_FLASH_ATTR tcpRecvCb(void* arg, char* data, uint16_t len)
{
  // uart0_sendStr("tcpRecvCb\r\n");

  // parse http
  parseHttp(data, len);
}

void ICACHE_FLASH_ATTR tcpReconCb(void* arg, int8_t err)
{
  // Nothing
  ;
}

void ICACHE_FLASH_ATTR tcpDisconCb(void* arg)
{
  if (tcpServer->conn->state == ESPCONN_NONE || tcpServer->conn->state == ESPCONN_CLOSE)
  {
    // uart0_sendStr("tcpDisconCb\r\n");

    os_free(tcpServer->conn);
    tcpServer->conn = NULL;
  }
}

void ICACHE_FLASH_ATTR tcpConnectCb(void * arg)
{
  // uart0_sendStr("tcpConnectCb\r\n");

  struct espconn* conn = arg;

  // init server data struct
  if (tcpServer->conn != NULL)
    espconn_disconnect(tcpServer->conn);

  tcpServer->conn = conn;
  tcpServer->txBufferLen = 0;
  tcpServer->rxBufferLen = 0;
  tcpServer->readyFlag = true;

  espconn_regist_recvcb(tcpServer->conn, tcpRecvCb);
  espconn_regist_reconcb(tcpServer->conn, tcpReconCb);
  espconn_regist_disconcb(tcpServer->conn, tcpDisconCb);
  espconn_regist_sentcb(tcpServer->conn, tcpSentCb);
}

void ICACHE_FLASH_ATTR startHttpServer()
{
  // assume AP is working
  httpStat = HTTP_IDLE;

  // start TCP listening
  tcpServer = (TCP_SERVER*)os_zalloc(sizeof(TCP_SERVER));
  tcpConn = (struct espconn*)os_zalloc(sizeof(struct espconn));
  espTcp = (esp_tcp*)os_zalloc(sizeof(esp_tcp));
  os_memset(tcpServer, 0, sizeof(TCP_SERVER));
  os_memset(tcpConn, 0, sizeof(struct espconn));
  os_memset(espTcp, 0, sizeof(esp_tcp));

  tcpConn->type = ESPCONN_TCP;
  espTcp->local_port = 80;
  tcpConn->proto.tcp = espTcp;
  tcpServer->conn = NULL;
  tcpServer->txBuffer = (char*)os_zalloc(MAX_TX_BUFFER);
  os_memset(tcpServer->txBuffer, 0, MAX_TX_BUFFER);
  tcpServer->rxBuffer = (char*)os_zalloc(MAX_RX_BUFFER);
  os_memset(tcpServer->rxBuffer, 0, MAX_RX_BUFFER);

  espconn_regist_connectcb(tcpConn, tcpConnectCb);
  espconn_accept(tcpConn);
  espconn_regist_time(tcpConn, SERVER_TIMEOUT, 0);
}

void ICACHE_FLASH_ATTR stopHttpServer()
{
  // set flash_param->staFlag
  flash_param_t* flash_param = flash_param_get();
  flash_param->staFlag = 1;
  flash_param_set();

  // reset
  os_delay_us(10000);
  system_restart();
}

void ICACHE_FLASH_ATTR resetHttpServer()
{
  // reset flash_param->staFlag
  flash_param_t* flash_param = (flash_param_t*)flash_param_get();
  flash_param->staFlag = 0;
  flash_param_set();

  // reset
  os_delay_us(10000);
  system_restart();
}

void ICACHE_FLASH_ATTR sendBuffer(TCP_SERVER* tcpServer, char* buffer, uint16_t len)
{
  // uart0_sendStr("sendBuffer\r\n");
  if (tcpServer->txBufferLen + len > MAX_TX_BUFFER)
    return;

  os_memcpy(tcpServer->txBuffer + tcpServer->txBufferLen, buffer, len);
  tcpServer->txBufferLen += len;

  sendTxBuffer(tcpServer);
}

void ICACHE_FLASH_ATTR parseHttp(char* data, uint16_t len)
{
  // uart0_sendStr("parseHttp\r\n");

  os_memcpy((tcpServer->rxBuffer + tcpServer->rxBufferLen), data, len);
  tcpServer->rxBufferLen += len;
  char* line = getLine();

  /*
  uart0_sendStr("getLine - ");
  uart0_sendStr(line);
  uart0_sendStr("\r\n");
  */

  // status
  while (line != NULL)
  {
    /*
    uart0_sendStr("newLine - ");
    uart0_sendStr(line);
    uart0_sendStr("\r\n");
    */

    if (line[0] == 'G' && 
      line[1] == 'E' &&
      line[2] == 'T')
    {
      // get header
      httpStat = HEADER_GET;
      // uart0_sendStr("HEADER_GET\r\n");
    }
    else if (line[0] == 'P' && 
      line[1] == 'O' &&
      line[2] == 'S' &&
      line[3] == 'T')
    {
      // post header
      httpStat = HEADER_POST;
      // uart0_sendStr("HEADER_POST\r\n");
    }
    else if (line[0] == '\r' && 
      line[1] == '\n')
    {
      // uart0_sendStr("blank line\r\n");
      // blank line
      if (httpStat == HEADER_GET)
      {
        httpStat = DONE_GET;
        // uart0_sendStr("DONE_GET\r\n");
      }
      else
      {
        httpStat = BODY_POST;
        // uart0_sendStr("BODY_POST\r\n");
      }
    }
    else
    {
      if (httpStat == BODY_POST)
      {
        // switch to done
        httpStat = DONE_POST;
        // uart0_sendStr("DONE_POST\r\n");
      }
    }

    // execute
    if (httpStat == DONE_GET)
    {
      // send page
      // uart0_sendStr("Entering HEADER_GET handler\r\n");
      respondHttp(true);
      break;
    }
    else if (httpStat == DONE_POST)
    {
      // read POST body, create HTTP_REQ
      httpReq = getHttpReq(line);

      /*
      uart0_sendStr("httpReq->uuid: \r\n");
      uart0_sendStr(httpReq->uuid);
      uart0_sendStr("\r\n");
      */

      // handle POST
      respondHttp(false);

      // set MQTT
      flash_param_t* flash_param = flash_param_get();
      
      os_memset(flash_param->clientId, 0, sizeof(flash_param->clientId));
      os_memset(flash_param->user, 0, sizeof(flash_param->user));
      os_memset(flash_param->pass, 0, sizeof(flash_param->pass));

      os_strncpy(flash_param->clientId, httpReq->uuid, os_strlen(httpReq->uuid));
      os_strncpy(flash_param->user, httpReq->uuid, os_strlen(httpReq->uuid));
      os_strncpy(flash_param->pass, httpReq->token, os_strlen(httpReq->token));
      flash_param_set();

      /*
      uart0_sendStr("flash_param->pass: \r\n");
      uart0_sendStr(flash_param->pass);
      uart0_sendStr("\r\n");
      */

      // set station
      struct station_config* staConf = (struct station_config*)os_zalloc(sizeof(struct station_config));
      os_bzero(staConf, sizeof(struct station_config));
      wifi_station_get_config_default(staConf);
      os_strncpy(staConf->ssid, httpReq->ssid, os_strlen(httpReq->ssid));
      os_strncpy(staConf->password, httpReq->pwd, os_strlen(httpReq->pwd));

      /*
      uart0_sendStr("staConf->ssid: \r\n");
      uart0_sendStr(staConf->ssid);
      uart0_sendStr("\r\n");
      */

      wifi_set_opmode(STATION_MODE);
      // write to system flash area
      ETS_UART_INTR_DISABLE();
      wifi_station_set_config(staConf);
      ETS_UART_INTR_ENABLE();

      os_free(httpReq);

      // set flash_param->staFlag and restart
      stopHttpServer();
    }

    os_free(line);
    line = getLine();
  }
}

void ICACHE_FLASH_ATTR respondHttp(bool getFlag)
{
  if (getFlag)
  {
    // uart0_sendStr("respondHttp - get \r\n");
    
    // create content
    char* content = (char*)os_zalloc(MAX_TX_BUFFER);
    content[0] = 0;
    os_strcat(content, "<html>\r\n");
    os_strcat(content, "<body>\r\n");
    os_strcat(content, "<h1>\r\n");
    os_strcat(content, "Hello, ESP Web\r\n");
    os_strcat(content, "</h1>\r\n");
    os_strcat(content, "<form action=\"setup\" method=\"post\">\r\n");
    os_strcat(content, "<p>SSID: <input type=\"text\" name=\"ssid\"></p>\r\n");
    os_strcat(content, "<p>Password: <input type=\"text\" name=\"pwd\"></p>\r\n");
    os_strcat(content, "<p>UUID: <input type=\"text\" name=\"uuid\"></p>\r\n");
    os_strcat(content, "<p>Token: <input type=\"text\" name=\"token\"></p>\r\n");
    os_strcat(content, "<input type=\"submit\" value=\"OK\">\r\n");
    os_strcat(content, "</form>\r\n");
    os_strcat(content, "</body>\r\n");
    os_strcat(content, "</html>\r\n");
    
    // create header
    char* header = getHeader(os_strlen(content));

    // assemble the page
    char* page = (char*)os_zalloc(MAX_TX_BUFFER);
    page[0] = 0;
    os_strcat(page, header);
    os_strcat(page, content);

    // send page
    sendBuffer(tcpServer, page, os_strlen(page));

    // collect resources
    os_free(header);
    os_free(page);
  }
  else
  {
    // create content
    char* content = (char*)os_zalloc(MAX_TX_BUFFER);
    content[0] = 0;
    os_strcat(content, "<html>\r\n");
    os_strcat(content, "<body>\r\n");
    os_strcat(content, "<h1>\r\n");
    os_strcat(content, "You're done!\r\n");
    os_strcat(content, "</h1>\r\n");
    os_strcat(content, "</body>\r\n");
    os_strcat(content, "</html>\r\n");
    
    // create header
    char* header = getHeader(os_strlen(content));

    // assemble the page
    char* page = (char*)os_zalloc(MAX_TX_BUFFER);
    page[0] = 0;
    os_strcat(page, header);
    os_strcat(page, content);

    // send page
    sendBuffer(tcpServer, page, os_strlen(page)); 
    
    // collect resources
    os_free(header);
    os_free(page);
  }
}

char* ICACHE_FLASH_ATTR getHeader(uint16_t contentLen)
{
  // create header buffer
  char* header = (char*)os_zalloc(100);
  header[0] = '\0';
  os_strcat(header, "HTTP/1.1 200 OK\r\n");
  
  // content-length
  char* lenStr = (char*)os_zalloc(30);
  os_sprintf(lenStr, "Content-Length: %d\r\n", contentLen);
  os_strcat(header, lenStr);
  os_strcat(header, "\r\n");

  // collect resources
  os_free(lenStr);

  return header;
}

char* ICACHE_FLASH_ATTR getLine()
{
  uint16_t lineLen;
  for (lineLen = 1; lineLen < tcpServer->rxBufferLen; lineLen++)
  {
    if (tcpServer->rxBuffer[lineLen - 1] == '\r' && 
      tcpServer->rxBuffer[lineLen] == '\n' ||
      lineLen == tcpServer->rxBufferLen - 1)
    {
      // gotcha
      char* line = (char*)os_zalloc(lineLen + 2);
      line[lineLen + 1] = '\0';
      os_memcpy(line, tcpServer->rxBuffer, lineLen + 1);

      uint16_t poolLen = MAX_RX_BUFFER - (lineLen + 1);
      char* pool = (char*)os_zalloc(poolLen);
      os_memcpy(pool, tcpServer->rxBuffer + (lineLen + 1), poolLen);
      os_memcpy(tcpServer->rxBuffer, pool, poolLen);
      os_free(pool);
      tcpServer->rxBufferLen -= lineLen + 1;

      return line;
    }
  }

  return NULL;
}

HTTP_REQ* ICACHE_FLASH_ATTR getHttpReq(char* line)
{
  if (httpReq != NULL)
    os_free(httpReq);
  httpReq = (HTTP_REQ*)os_zalloc(sizeof(HTTP_REQ));
  
  char* ssid = (char*)((int)os_strchr(line, '=') + 1);
  int ssidLen = (int)os_strchr(ssid, '&') -(int)ssid;

  char* pwd = (char*)((int)os_strchr(ssid, '=') + 1);
  int pwdLen = (int)os_strchr(pwd, '&') - (int)pwd;

  char* uuid = (char*)((int)os_strchr(pwd, '=') + 1);
  int uuidLen = (int)os_strchr(uuid, '&') - (int)uuid;

  char* token = (char*)((int)os_strchr(uuid, '=') + 1);
  int tokenLen = (int)os_strlen(token);

  os_memset(httpReq->ssid, 0, sizeof(httpReq->ssid));
  os_memset(httpReq->pwd, 0, sizeof(httpReq->pwd));
  os_memset(httpReq->uuid, 0, sizeof(httpReq->uuid));
  os_memset(httpReq->token, 0, sizeof(httpReq->token));

  os_memcpy(httpReq->ssid, ssid, ssidLen);
  os_memcpy(httpReq->pwd, pwd, pwdLen);
  os_memcpy(httpReq->uuid, uuid, uuidLen);
  os_memcpy(httpReq->token, token, tokenLen);

  /*
  uart0_sendStr("token: \r\n");
  uart0_sendStr(token);
  uart0_sendStr("\r\n");

  uart0_sendStr("httpReq->token: \r\n");
  uart0_sendStr(httpReq->token);
  uart0_sendStr("\r\n");
  */

  return httpReq;
}

