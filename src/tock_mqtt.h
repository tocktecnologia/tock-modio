#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "utils.h"

// Update these with values suitable for your network.

String mqtt_server = "192.168.0.32";
String pub_topic = "tock-commands";
String sub_topic = "tock-commands";
String broker_user = "tocktec.com.br";
String broker_pass = "tock30130tecnologia";

long lastReconnectAttempt = 0;
long timeToConnect = 5000;
int countWiFiDisconnection = 0;

WiFiClient espClient;
PubSubClient client(espClient);

DynamicJsonDocument configJsonCache(FILE_CONFIG_SIZE);

void updateStateConfig(int hardwarePin, int state)
{

    JsonArray pinsArray = configJsonCache["configs"]["pins"].as<JsonArray>();
    for (int i = 0; i < pinsArray.size(); i++)
    {
        if (configJsonCache["configs"]["pins"][i]["hardwarePin"].as<int>() == hardwarePin)
            configJsonCache["configs"]["pins"][i]["state"] = state;
    }

    // NEED SERIALIZATION AND DESERIALIZATION O.o
    String configJsonStr;
    serializeJson(configJsonCache, configJsonStr);
    deserializeJson(configJsonCache, configJsonStr);
    // Serial.println(configJsonStr);
}

DynamicJsonDocument reportStates()
{
    DynamicJsonDocument reportedMessageJson(384);

    JsonArray pinsArray = configJsonCache["configs"]["pins"].as<JsonArray>();
    for (int i = 0; i < pinsArray.size(); i++)
    {
        auto softwarePin = configJsonCache["configs"]["pins"][i]["softwarePin"].as<int>();
        auto key = "pin" + String(softwarePin);
        auto state = configJsonCache["configs"]["pins"][i]["state"].as<int>();

        if (!reportedMessageJson["state"]["reported"].containsKey(key)) // isso evitar sobrescrita de estados quando o pin cofigurado é o mesmo pra duas saidas diferentes
            reportedMessageJson["state"]["reported"][key] = state;
    }

    String reportedMessage;
    serializeJson(reportedMessageJson, reportedMessage);
    client.publish(pub_topic.c_str(), reportedMessage.c_str());
    return reportedMessageJson;
}

int getPinIdOutput(int softwarePin)
{

    if (!configJsonCache.containsKey("configs"))
    {
        Serial.println("\nconfigJson NOT contains key 'configs'");
        return -1;
    }
    if (!configJsonCache["configs"].containsKey("pins"))
    {
        Serial.println("\nconfigs NOT contains key 'pins'");
        return -1;
    }

    JsonArray pinsArray = configJsonCache["configs"]["pins"].as<JsonArray>();
    for (JsonVariant pin : pinsArray)
    {
        if (pin["softwarePin"].as<int>() == softwarePin)
            return pin["hardwarePin"].as<int>();
    }
    return -1;
}

boolean reconnectMqtt()
{

    if (WiFi.status() != WL_CONNECTED)
    {
        countWiFiDisconnection++;
        // String ssid = configJsonCache["configs"]["ssid"].as<String>();
        // String password = configJsonCache["configs"]["password"].as<String>();
        // if (countWiFiDisconnection >= 3)
        //     setup_wifi(ssid.c_str(), password.c_str());
    }

    String thingName = deviceID();
    String clientId = "TOCK-" + thingName;
    Serial.print(String(" Try connecting to ") + mqtt_server + String(" ...\n"));

    if (client.connect(clientId.c_str(), broker_user.c_str(), broker_pass.c_str()))
    {
        Serial.println(clientId + String("connected. Broker ip: ") + mqtt_server + "!");
        client.publish(pub_topic.c_str(), (thingName + String(" connected")).c_str());
        // reportStates();
        client.subscribe(sub_topic.c_str());
    }

    return client.connected();
}

void paramsSetup()
{
    if (configJsonCache["configs"].containsKey("broker"))
    {
        mqtt_server = configJsonCache["configs"]["broker"].as<String>();
    }
    if (configJsonCache["configs"].containsKey("broker_user"))
    {
        broker_user = configJsonCache["configs"]["broker_user"].as<String>();
    }
    if (configJsonCache["configs"].containsKey("broker_pass"))
    {
        broker_pass = configJsonCache["configs"]["broker_pass"].as<String>();
    }
}

void callback(char *topic, byte *payload, unsigned int length)
{

    String payloadString = "";
    for (unsigned int i = 0; i < length; i++)
    {
        payloadString = payloadString + (char)payload[i];
    }

    StaticJsonDocument<256> fileJson;
    JsonObject fileJsonObj;
    deserializeJson(fileJson, payloadString);

    if (!fileJson["state"].containsKey("desired"))
        return;

    Serial.println("Message desired arrived: " + payloadString);
    fileJsonObj = fileJson.as<JsonObject>()["state"]["desired"];
    serializeJsonPretty(fileJson, Serial); // debug
    fileJson.clear();

    // iterate over the messsage desired
    for (JsonPair jsonPair : fileJsonObj)
    {

        int softwarePin = String(jsonPair.key().c_str()).substring(3).toInt(); // jsonPair.key = "pin12" -> result = 12
        int hardwarePin = getPinIdOutput(softwarePin);
        Serial.println("\nhardwarePin: " + String(hardwarePin));

        if (hardwarePin >= 0)
        {

            // update pin output
            String state = jsonPair.value();
            if (state != "1" && state != "0")
                state = (String)digitalRead(hardwarePin);

            pinMode(hardwarePin, OUTPUT);
            digitalWrite(hardwarePin, !state.toInt());

            // // UPDATE configs
            updateStateConfig(hardwarePin, state.toInt());
            writeJsonMem(configJsonCache, FILE_CONFIG);
            reportStates();
        }
    }
}

void mqttSetup(ArduinoJson6200_F1::DynamicJsonDocument &configJson)
{
    // // NEED THIS
    String configJsonStr;
    serializeJson(configJson, configJsonStr);
    deserializeJson(configJsonCache, configJsonStr);

    paramsSetup();

    client.setServer(mqtt_server.c_str(), 1883);
    client.setCallback(callback);
    lastReconnectAttempt = 0;
}

void mqttLoop()
{

    if (!client.connected())
    {
        long now = millis();
        if (now - lastReconnectAttempt > timeToConnect)
        {
            lastReconnectAttempt = now;
            // Attempt to reconnect
            if (reconnectMqtt())
            {
                lastReconnectAttempt = 0;
            }
        }
    }
    else
    {
        // Client connected
        client.loop();
    }
}
