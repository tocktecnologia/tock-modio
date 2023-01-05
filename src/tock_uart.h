
#ifndef UART
#define UART
#include <Arduino.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>
#include "utils.h"
#include "pins.h"

#define FILE_CONFIG "/config_pins.json"
#define FILE_CONFIG_SIZE 2048

ArduinoJson6200_F1::DynamicJsonDocument retrieveConfigFromMemory()
{
    // Serial.println("Formating ...");
    // SPIFFS.format();

    DynamicJsonDocument configJson(FILE_CONFIG_SIZE);

    Serial.println("mounting FS ...");
    if (SPIFFS.begin())
    {
        Serial.println("mounted file system.");
        if (SPIFFS.exists(FILE_CONFIG))
        {
            Serial.println("reading config file");
            File configFile = SPIFFS.open(FILE_CONFIG, "r");
            if (configFile)
            {
                size_t size = configFile.size();
                std::unique_ptr<char[]> buf(new char[size]);
                configFile.readBytes(buf.get(), size);

                auto deserializeError = deserializeJson(configJson, buf.get());
                if (!deserializeError)
                {
                    wroom32ConfigPins(configJson);
                    serializeJson(configJson, Serial);
                }
                else
                    Serial.println("failed to load json config");

                configFile.close();
            }
        }
        else
            Serial.println("File not exist yet.");
    }
    else
        Serial.println("failed to mount FS.");

    return configJson;
}

void writeJsonMemory(DynamicJsonDocument &configJson)
{

    // SPIFFS.remove(FILE_CONFIG);

    // writing
    File configFile = SPIFFS.open(FILE_CONFIG, "w");
    if (!configFile)
    {
        Serial.println("failed to open config file for writing");
    }
    // Serial.println("writing in memory: \n");
    // serializeJson(messageJson, Serial);
    serializeJson(configJson, configFile);
    configFile.close();
}

void configPins(DynamicJsonDocument &configJson)
{

    // extract the values
    JsonArray pinsArray = configJson["configs"]["pins"].as<JsonArray>();
    for (JsonVariant pin : pinsArray)
    {

        // Serial.println("software Pin: " + pin["softwarePin"].as<String>());
        if (pin["mode"].as<String>().equals("out"))
        {
            pinMode(pin["hardwarePin"].as<int>(), OUTPUT);
            digitalWrite(pin["hardwarePin"].as<int>(), !pin["state"].as<int>());
        }
        else if (pin["mode"].as<String>().equals("out"))
            pinMode(pin["hardwarePin"].as<int>(), INPUT);
    }
}

void readSerial(DynamicJsonDocument &configJson)
{
    if (Serial.available())
    {
        // CHECK WORD CODE
        String payloadString = Serial.readString();
        if (payloadString[payloadString.length() - 1] == '#' && payloadString[payloadString.length() - 2] == '#')
        {
            payloadString = payloadString.substring(0, payloadString.length() - 2);
            Serial.println("inputString :" + String(payloadString));

            // DynamicJsonDocument configJson(FILE_CONFIG_SIZE);
            deserializeJson(configJson, payloadString);

            // CONFIG
            if (configJson.containsKey("configs"))
            {

                // CHECK id
                if (configJson["configs"]["id"].as<String>().equals(deviceID()))
                {
                    configPins(configJson);
                    writeJsonMemory(configJson);
                    delay(100);
                    ESP.restart();
                }
                else
                    Serial.println("Wrong Device ID!");
            }
        }
    }
}

#endif
