#include <WiFi.h>

const char *ssid = "Unknown";
const char *password = "1234567890asdf";

const char *StateToStr[] = {"WL_IDLE_STATUS", "WL_NO_SSID_AVAIL", "WL_SCAN_COMPLETED", "WL_CONNECTED", "WL_CONNECT_FAILED", "WL_CONNECTION_LOST", "WL_DISCONNECTED"};

void setup()
{
    Serial.begin(115200);
    Serial.println();

    WiFi.begin(ssid, password);
    WiFi.setSleep(false); // this code solves my problem

    delay(10000);

        Serial.println("Connecting");
    int i;
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.printf("%s status: %s \n", i % 4 == 0 ? "-" : ".", StateToStr[WiFi.status()]);
        i++;
    }
    Serial.println();

    Serial.print("Connected, IP address: ");
    Serial.println(WiFi.localIP());
}

void loop() {}