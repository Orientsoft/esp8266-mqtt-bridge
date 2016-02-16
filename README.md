# esp8266-mqtt-bridge
esp8266-mqtt-bridge creates a transparent MQTT-Serial bridge between MQTT broker and serial port of ESP8266.  
For example, You could connect Arduino and ESP8266 with serial port. Through serial port, Arduino could communicate with MQTT broker.  
Web based setup interface
-------------------------
When powered on for the first time, esp8266-mqtt-bridge sets the ESP wifi to AP mode and provides a simple setup portal.  
To enter the setup portal, connect the AP (SSID: ESP_CONF, password: orientsoft), then go to browser and visit 192.168.4.1.  
You could setup SSID and password to be connected by the bridge (when setup is done), UUID and Token of your device (register the device on borgnix.com to get your UUID/Token).  
Click OK button and the parameters will be saved to ESP flash. The bridge will automatically reset, go to wifi station mode and connect to MQTT broker on borgnix.com.  
Restful setup interface
-----------------------
For APP developers, restful setup interface is also working.  
Simply HTTP POST your SSID/password/UUID/Token to 192.168.4.1 and the bridge will respond status 200.  
eg.  
<pre><code>ssid=MY_AP_SSID&pwd=MY_AP_PWD&uuid=MY_DEVICE_UUID&token=MY_DEVICE_TOKEN  </pre></code>
Actually, the bridge doesn't care about the parameter name, so you should keep the sequence of the parameters, as shown in above example.  
Usage
-----
After setup, the bridge will connect to your AP and subscribe to /dev/YOUR_UUID/out topic on the borgnix.com MQTT broker.  
Once connected to MQTT broker, GPIO2 of ESP8266 will be pulled low so you could use an LED to indicate the connection status.  
Any data published to /dev/YOUR_UUID/out topic will be redirected to ESP serial port. So you could simply read data from your device's serial port.  
Also, you could write data to your device's serial port, and the bridge will publish your data to /dev/YOUR_UUID/in topic.  
Reset parameters
----------------
To reset the parameters, you could pull low the GPIO13 pin of ESP8266 (ESP-01 doesn't have this pin, so you should use this function on ESP-12F). The parameters will be cleared from flash and the bridge will enter AP mode providing setup interfaces.  
