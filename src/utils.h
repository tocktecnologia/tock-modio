#ifndef UTILS
#define UTILS

#include <WiFi.h>
#include <Ticker.h>
#include <FS.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>

Ticker flipper;
void flip()
{
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN)); // set pin to the opposite state
}

String deviceID()
{
    byte mac[6];
    WiFi.macAddress(mac);
    return String(mac[0], HEX) + String(mac[1], HEX) + String(mac[2], HEX) + String(mac[3], HEX) + String(mac[4], HEX) + String(mac[5], HEX);
}
String getClientID() { return "TOCK-" + deviceID(); }

bool tockDebug = false;
void tockPrint(String str)
{
    if (tockDebug)
    {
        Serial.print("TOCK:");
        Serial.println(str);
    }
}

// FILE SYSTEM
String readFile(fs::FS &fs, const char *path)
{
    File file = fs.open(path, "r+");
    if (!file || file.isDirectory())
    {
        return String();
    }
    String fileContent;
    while (file.available())
    {
        fileContent += String((char)file.read());
    }
    file.close();
    // Serial.println(fileContent);
    return fileContent;
}
String writeFile(fs::FS &fs, const char *path, const char *message)
{
    File file = fs.open(path, "w");
    if (!file)
    {
        return String();
    }
    if (file.print(message))
    {
        file.close();
        return String(message);
    }
    else
    {
        file.close();
        return String();
    }
}

void writeJsonMem(DynamicJsonDocument &json, const char *filename)
{

    // SPIFFS.remove(FILE_CONFIG);

    // writing
    File configFile = SPIFFS.open(filename, "w");
    if (!configFile)
    {
        Serial.println("failed to open config file for writing");
    }
    // Serial.println("writing in memory: \n");
    // serializeJson(messageJson, Serial);
    serializeJson(json, configFile);
    configFile.close();
}

#endif
