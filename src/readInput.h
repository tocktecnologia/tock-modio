
std::vector<int> lastStates;
void setupReadingInputs()
{
    // read all pins
    for (unsigned int i = 0; i < wifiParams.size(); i++)
    {
        if (String(wifiParams[i]->getLabel()).startsWith("GPIO"))
        {
            lastStates.push_back(wifiParams[i]->getLabelPlacement()); // we used getLabelPlacement as state of pin :)
        }
        else
        {
            lastStates.push_back(-1);
        }
    }
}

void loopReadingInputs()
{
    if (!String(custom_io.getValue()).equals("i"))
        return;

    // read all pins
    for (unsigned int i = 0; i < wifiParams.size(); i++)
    {
        if (String(wifiParams[i]->getLabel()).startsWith("GPIO"))
        {
            // if state change 1 to 0
            // int currentPin = String(wifiParams[i]->getID()).toInt();
            int HardwarePin = String(wifiParams[i]->getID()).toInt();
            int MQTTPin = String(wifiParams[i]->getValue()).toInt();
            if (lastStates[i] == 1 && digitalRead(HardwarePin) == 0)
            {
                delay(100);
                if (lastStates[i] == 1 && digitalRead(HardwarePin) == 0)
                {
                    lastStates[i] = 0;
                    // publish mqtt
                    String message = "{\"state\":{\"desired\":{\"pin" + String(MQTTPin) + "\":x}}}";
                    client.publish(pub_topic, message.c_str());
                    delay(500);
                }
            }
            // if state change 0 to 1re
            else if (lastStates[i] == 0 && digitalRead(HardwarePin) == 1)
            {
                lastStates[i] = 1;
            }
            // else do nothing
        }
    }
}
