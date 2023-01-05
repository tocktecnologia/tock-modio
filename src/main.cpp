//
// #include "TockMQTT.h"
// #include "otaUpdatePubSub.h"
#define LED_BUILTIN 32 // GPIO 2

#include "tock_wm.h"
#include "tock_uart.h"
#include "tock_mqtt.h"

DynamicJsonDocument configJson(FILE_CONFIG_SIZE);

void setup()
{
  Serial.begin(9600);
  Serial.println(String("\n\n") + String(deviceID()) + " Started!");
  // pinMode(LED_BUILTIN, OUTPUT);
  // flipper.attach(0.1, flip); // pisca rapido enquanto tenta se conectar ao wifi

  configJson = retrieveConfigFromMemory();
  String configJsonStr;
  serializeJson(configJson, configJsonStr);
  deserializeJson(configJson, configJsonStr);

  setupWM();

  mqttSetup(configJson);
}

void loop()
{

  readSerial(configJson);
  loopWM();
  mqttLoop();
}
