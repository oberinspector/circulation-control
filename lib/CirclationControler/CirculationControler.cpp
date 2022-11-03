//
// Created by thomas on 24.09.2022.
//
#include "CirculationControler.h"

int lastCirculationPumpState = PUMP_OFF;

unsigned long lastMillis = 0;
String lastValveStates = "|-----|-----|-----|-----|-----|-----|--|";
// circulation pin and timer configuration
// W1: Wohnung 1, B: Bad, K: Küche, G: Gästezimmer, X: Bad und Küche
Tap taps[] =
        {{"W1-B", W1_B_BUTTON, W1_B_VALVE, 10},
         {"W1-K", W1_K_BUTTON, W1_K_VALVE, 20},
         {"W1-G", W1_G_BUTTON, W1_G_VALVE, 30},
         {"W2-X", W2_X_BUTTON, W2_X_VALVE, 30},
         {"W3-B", W3_B_BUTTON, W3_B_VALVE, 15},
         {"W3-K", W3_K_BUTTON, W3_k_VALVE, 30}
        };
unsigned long timeouts[] = {0, 0, 0, 0, 0, 0};

void CirculationControler::initCirculationController() {
    pinMode(CIRCULATION_PUMP_PIN, OUTPUT);
    digitalWrite(CIRCULATION_PUMP_PIN, PUMP_OFF);

    for (Tap tap: taps) {
        pinMode((tap.valve), OUTPUT);
        digitalWrite(tap.valve, CLOSE);
        pinMode(tap.button, INPUT_PULLUP);
        Serial.println(tap.name + " initialized");
    }
}

void CirculationControler::handleTaps() {
    unsigned long currentMillis = millis();
    if (lastMillis > currentMillis) {
        // handle overflow
        lastMillis = currentMillis;
        // reset all falves
        initCirculationController();
        Serial.println("Timer Overfrlow detected... reset all timers");
        return;
    }

    int currentCirculationPumpState = PUMP_OFF;
    String currentValveStates = "|";

    for (int i = 0; i < 6; ++i) {
        Tap tap = taps[i];

        // check active taps
        // check for tap activation
        if (timeouts[i] == 0 && (digitalRead(tap.button) == BUTTON_PRESSED)) {
            timeouts[i] = currentMillis + tap.circulationTime * 1000;
            digitalWrite(tap.valve, OPEN);
            currentCirculationPumpState = PUMP_ON;
            currentValveStates += tap.name + "|";
            Serial.println(tap.name + ": Circulation startet at:" + currentMillis + ". Stop at: " + timeouts[i]);
            continue;
        }

        if (timeouts[i] != 0 && timeouts[i] < currentMillis) {
            timeouts[i] = 0;
            digitalWrite(tap.valve, CLOSE);
            currentValveStates += "----|";
            Serial.println(tap.name + ": Circulation stopped at:" + currentMillis);
            continue;
        }

        if (timeouts[i] != 0 && timeouts[i] > currentMillis) {
            currentCirculationPumpState = PUMP_ON;
            currentValveStates += tap.name + "|";
        } else {
            currentValveStates += "----|";
        }
    }

    if (lastCirculationPumpState != currentCirculationPumpState) {
        digitalWrite(CIRCULATION_PUMP_PIN, currentCirculationPumpState);
        lastCirculationPumpState = currentCirculationPumpState;
    }

    if (currentValveStates != lastValveStates) {
        if (currentCirculationPumpState == PUMP_ON) {
            currentValveStates += "ZP|";
        } else {
            currentValveStates += "--|";
        }
    }
    if (currentValveStates != lastValveStates) {
        lastValveStates = currentValveStates;
        Serial.println(lastValveStates);
    }
    lastMillis = currentMillis;
 }
