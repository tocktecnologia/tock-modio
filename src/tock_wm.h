#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include "utils.h"

WiFiManager wm;

void setupWM()
{
  WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
                       // wm.resetSettings() ;
  wm.setConfigPortalBlocking(false);
  wm.setConfigPortalTimeout(60);
  wm.setConnectTimeout(15);
  wm.setConfigPortalTimeoutCallback([]
                                    { wm.reboot(); });

  if (wm.autoConnect(getClientID().c_str(), "tock1234"))
  {
    Serial.println("Wifi connected!");
  }
  else
  {
    Serial.println("Configportal running");
  }
}

void loopWM()
{
  wm.process();
  // put your main code here, to run repeatedly:
}