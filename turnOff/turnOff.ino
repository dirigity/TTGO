#define LILYGO_WATCH_2020_V1 

#include <LilyGoWatch.h>

TTGOClass *ttgo;

void setup() {

  Serial.begin(115200);

  Serial.printf("starting sleep");

  ttgo = TTGOClass::getWatch();
  ttgo->begin();
  ttgo->openBL();
  ttgo->displaySleep();

  ttgo->powerOff();

  ttgo->power->setPowerOutPut(AXP202_LDO3, false);
  ttgo->power->setPowerOutPut(AXP202_LDO4, false);
  ttgo->power->setPowerOutPut(AXP202_LDO2, false);
  ttgo->power->setPowerOutPut(AXP202_EXTEN, false);
  ttgo->power->setPowerOutPut(AXP202_DCDC2, false);

  esp_deep_sleep_start();

  Serial.printf("sleeping...");

}

void loop() {
  // put your main code here, to run repeatedly:

}
