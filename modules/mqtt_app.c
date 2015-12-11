#include "c_types.h"
#include "user_interface.h"
#include "mem.h"
#include "osapi.h"
#include "config.h"
#include "flash_param.h"

#include "mqtt_app.h"

MQTT_Client* mqttClient;
static char mqtt_buffer[MAX_MQTT_BUFFER];
flash_param_t* param;

// pub topic
char* pubTopic;
bool mqttConnectionFlag = false;

uint32_t mqttConnectedCb(uint32_t* args)
{
  mqttConnectionFlag = true;

  pubTopic = (char*)os_zalloc(os_strlen(param->user) + 13);
  pubTopic[0] = 0;
  os_strcat(pubTopic, "/devices/");
  os_strcat(pubTopic, param->user);
  os_strcat(pubTopic, "/in");

  MQTT_Publish(mqttClient, pubTopic, "ESP RDY\r\n", 9, 0, 0);

  // uart0_sendStr("MQTT CONNECTED\r\n");
  LED_Set(1);

  // MQTT sub /devices/UUID/out
  char* subTopic = (char*)os_zalloc(os_strlen(param->user) + 14);
  subTopic[0] = 0;
  os_strcat(subTopic, "/devices/");
  os_strcat(subTopic, param->user);
  os_strcat(subTopic, "/out");

  // qos = 0
  MQTT_Subscribe(mqttClient, subTopic, 0);
  
  os_free(subTopic);
}

void mqttDisconnectedCb(uint32_t* args)
{
  mqttConnectionFlag = false;

  // uart0_sendStr("MQTT DISCONNECTED\r\n");
  LED_Set(0);
}

void mqttPublishedCb(uint32_t* args)
{
  // uart0_sendStr("MQTT Published\r\n");
}

void mqttDataCb(uint32_t* args,
  const char* topic,
  uint32_t topicLen,
  const char* data,
  uint32_t dataLen)
{
  uint16_t crc = 0;
  MQTT_Client* client = (MQTT_Client*)args;

  #ifdef CONFIG_DYNAMIC
  if (dataLen >= 5 && 
    data[0] == '+' && 
    data[1] == '+' && 
    data[2] == '+' && 
    data[3] =='A' && 
    data[4] == 'T')
  {
    // modify config_parse so it fills our struct
    config_parse(data, dataLen);
  }
  else
  #endif
  
  uart0_tx_buffer(data, dataLen);
}

uint32_t ICACHE_FLASH_ATTR MqttSetup(uint8_t* clientId,
  uint8_t* user,
  uint8_t* pass,
  uint32_t keepalive,
  uint32_t cleanSession)
{
  // init mqtt client
  mqttClient = (MQTT_Client*)os_zalloc(sizeof(MQTT_Client));
  if (mqttClient == NULL)
    return 0;
  os_memset(mqttClient, 0, sizeof(MQTT_Client));

  MQTT_InitClient(mqttClient, clientId, user, pass, keepalive, cleanSession);
  // register callbacks
  mqttClient->connectedCb = (MqttCallback)mqttConnectedCb;
  mqttClient->disconnectedCb = (MqttCallback)mqttDisconnectedCb;
  mqttClient->publishedCb = (MqttCallback)mqttPublishedCb;
  mqttClient->dataCb = (MqttDataCallback)mqttDataCb;

  return (uint32_t)mqttClient;
}

uint32_t ICACHE_FLASH_ATTR MqttLwt(uint8_t* topic,
  uint8_t* message,
  uint32_t qos,
  uint32_t retain)
{
  uint16_t len;

  // topic
  if (mqttClient->connect_info.will_topic)
    os_free(mqttClient->connect_info.will_topic);
  len = os_strlen(topic);
  mqttClient->connect_info.will_topic = (uint8_t*)os_zalloc(len + 1);
  os_memcpy(mqttClient->connect_info.will_topic, topic, len);
  mqttClient->connect_info.will_topic[len] = 0;

  // message
  if (mqttClient->connect_info.will_message)
    os_free(mqttClient->connect_info.will_message);
  len = os_strlen(message);
  mqttClient->connect_info.will_message = (uint8_t*)os_zalloc(len + 1);
  os_memcpy(mqttClient->connect_info.will_message, message, len);
  mqttClient->connect_info.will_message[len] = 0;

  // qos & retain
  mqttClient->connect_info.will_qos = qos;
  mqttClient->connect_info.will_retain = retain;

  return 1;
}

uint32_t ICACHE_FLASH_ATTR MqttConnect(uint8_t* host,
  uint32_t port,
  uint32_t security)
{
  uint16_t len;

  // host
  if (mqttClient->host)
    os_free(mqttClient->host);
  len = os_strlen(host);
  mqttClient->host = (uint8_t*)os_zalloc(len + 1);
  os_memcpy(mqttClient->host, host, len);
  mqttClient->host[len] = 0;

  // port & sec
  mqttClient->port = port;
  mqttClient->security = security;

  MQTT_Connect(mqttClient);

  return 1;
}

void ICACHE_FLASH_ATTR initMqtt(flash_param_t* flashParam)
{
  #ifdef CONFIG_DYNAMIC
  MqttSetup(flashParam->clientId,
    flashParam->user,
    flashParam->pass,
    flashParam->keepalive,
    flashParam->cleanSession);
  
  uart0_sendStr(flashParam->clientId);
  uart0_sendStr("\r\n");
  uart0_sendStr(flashParam->user);
  uart0_sendStr("\r\n");
  uart0_sendStr(flashParam->pass);
  uart0_sendStr("\r\n");

  MqttLwt("/lwt", "offline", 0, 0);

  // no ssl
  MqttConnect(MQTT_HOST, MQTT_PORT, 0);
  // MqttConnect("z.borgnix.com", 1883, 0);

  #else
  MqttSetup(MQTT_CLIENTID,
    MQTT_USER,
    MQTT_PASS,
    MQTT_KEEPALIVE,
    MQTT_CLEANSESSION);

  MqttLwt("/lwt", "offline", 0, 0);

  // no ssl
  MqttConnect(MQTT_HOST,
    MQTT_PORT,
    0);
  #endif

  param = flashParam;
}
