#include <Arduino.h>
#include <../lib/CirclationControler/CirculationControler.h>
#include <../lib/FanControler/FanControler.h>

CirculationControler cc;
FanControler fc;

void setup() {
    //start serial connection for debug outputs
    Serial.begin(9600);
    cc.initCirculationController();
    fc.initFanController();
}

void loop() {
    cc.handleTaps();
    fc.handleFanControler();
}