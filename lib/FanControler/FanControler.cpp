//
// Created by thomas on 24.09.2022.
//

#include "FanControler.h"

boolean fanHumidityAutoMode = true;
FanState lastFanState = FAN_OFF;
unsigned long nextFanOffTimeout = 0;
unsigned long lastMs = 0;
unsigned long fanButtonPressedTime = 0;
boolean lastFanButtonState = BUTTON_STATE_RELEASED;

SimpleDHT11 dht11(DHT11_DATA_PIN);
unsigned long nextHumidityMeasurement = 0;
unsigned long humidityAutoPauseUntil = 0;
unsigned int humidityHighCounter = 0;

void FanControler::initFanController() {
    pinMode(BATH_FAN_SPEED_RELAYS_PIN, OUTPUT);
    pinMode(BATH_FAN_POWER_RELAYS_PIN, OUTPUT);
    pinMode(BATH_FAN_BUTTON, INPUT_PULLUP);
    lastFanState = FAN_OFF;
    updateFanAndStartTimer(lastFanState, 0);
    nextFanOffTimeout = 0;
    lastMs = millis();
    fanButtonPressedTime = 0;
    lastFanButtonState = BUTTON_STATE_RELEASED;
    nextHumidityMeasurement = 0;
    Serial.println("Fan controler initialized");
}

void FanControler::handleFanControler() {
    unsigned long currentMillis = millis();
    if (lastMs > currentMillis) {
        // handle overflow
        initFanController();
        lastMs = currentMillis;
        return;
    }
    handleHumidityFanControl(currentMillis);
    handleFanButtonEvent(currentMillis);
    lastMs = currentMillis;
}

void FanControler::handleFanButtonEvent(unsigned long currentMillis) {
    if (nextFanOffTimeout != 0 && currentMillis > nextFanOffTimeout) {
        lastFanState = FAN_OFF;
        updateFanAndStartTimer(lastFanState, FAN_DURATION_MS);
        return;
    }

    FanButtonEvent fanEvent = checkButton(currentMillis);

    switch (fanEvent) {
        case NONE:
            break;
        case FAN_BUTTON_SHORT:
            lastFanState = FAN_OFF;
            updateFanAndStartTimer(lastFanState, 0);
            break;
        case FAN_BUTTON:
            lastFanState = rotateFanState(lastFanState);
            updateFanAndStartTimer(lastFanState, FAN_DURATION_MS);
            break;
        case FAN_BUTTON_LONG:
            if (lastFanState == FAN_OFF) {
                lastFanState = rotateFanState(lastFanState);
                updateFanAndStartTimer(lastFanState, FAN_DURATION_MS);
            } else {
                increaseTimer();
            }
            break;
        case FAN_BUTTON_XLONG:
            fanHumidityAutoMode = !fanHumidityAutoMode;
            if (fanHumidityAutoMode) {
                // feedback DHT auto on: fan fast for 5 s
                lastFanState = FAN_S2;
                updateFanAndStartTimer(lastFanState, 5000);
            } else {
                // feedback DHT auto off: fan slow for 1 s
                lastFanState = FAN_S1;
                updateFanAndStartTimer(lastFanState, 1000);
            }
            break;
    }
}

FanButtonEvent FanControler::checkButton(unsigned long currentMillis) {
    if (digitalRead(BATH_FAN_BUTTON) == LOW && lastFanButtonState == BUTTON_STATE_RELEASED) {
        lastFanButtonState = BUTTON_STATE_PRESSED;
        fanButtonPressedTime = currentMillis;
        return NONE;
    }

    if (digitalRead(BATH_FAN_BUTTON) == HIGH && lastFanButtonState == BUTTON_STATE_PRESSED) {
        lastFanButtonState = BUTTON_STATE_RELEASED;

        unsigned long millisSincePressed = currentMillis - fanButtonPressedTime;
        fanButtonPressedTime = 0;

        if (millisSincePressed < FAN_BUTTON_TRESHHOLD_MS) {
            return NONE;
        }
        if (millisSincePressed < FAN_BUTTON_SHORT_MS) {
            Serial.println("Fan Button short pressed > stop fan");
            return FAN_BUTTON_SHORT;
        }

        if (millisSincePressed < FAN_BUTTON_LONG_MS) {
            Serial.println("Fan Button pressed > start fan, increase speed, off");
            return FAN_BUTTON;
        }

        if (millisSincePressed < FAN_BUTTON_XLONG_MS) {
            Serial.println("Fan Button pressed long > increase timer");
            return FAN_BUTTON_LONG;
        }

        if (millisSincePressed < 2 * FAN_BUTTON_XLONG_MS) {
            Serial.println("Fan Button pressed extra long (5s)  > toggle DHT automode");
            return FAN_BUTTON_XLONG;
        }
    }
    return NONE;
}

void stopTimer() {
    nextFanOffTimeout = 0;
}

void startTimer(unsigned long duration) {
    unsigned long currentMillis = millis();
    nextFanOffTimeout = currentMillis + duration;
}

void FanControler::increaseTimer() {
    if (nextFanOffTimeout != 0) {
        nextFanOffTimeout = nextFanOffTimeout + FAN_DURATION_MS;
    } else {
        nextFanOffTimeout = millis() + FAN_DURATION_MS;
    }
}

FanState FanControler::rotateFanState(FanState fanState) {
    switch (fanState) {
        case FAN_OFF:
            return FAN_S1;
        case FAN_S1:
            return FAN_S2;
        case FAN_S2:
            return FAN_OFF;
    }
    return FAN_OFF;
}

void FanControler::updateFanAndStartTimer(FanState state, unsigned long duration) {
    switch (state) {
        case FAN_OFF:
            digitalWrite(BATH_FAN_POWER_RELAYS_PIN, HIGH);
            digitalWrite(BATH_FAN_SPEED_RELAYS_PIN, HIGH);
            stopTimer();
            break;
        case FAN_S1:
            digitalWrite(BATH_FAN_POWER_RELAYS_PIN, LOW);
            digitalWrite(BATH_FAN_SPEED_RELAYS_PIN, HIGH);
            startTimer(duration);
            break;
        case FAN_S2:
            digitalWrite(BATH_FAN_POWER_RELAYS_PIN, LOW);
            digitalWrite(BATH_FAN_SPEED_RELAYS_PIN, LOW);
            startTimer(duration);
            break;
    }
}

void FanControler::handleHumidityFanControl(unsigned long currentMillis) {
    if (!fanHumidityAutoMode || currentMillis < nextHumidityMeasurement) {
        return;
    }
    nextHumidityMeasurement = currentMillis + HUMIDITY_CHECK_INTERVALL;

    // read without samples.
    byte temperature = 0;
    byte humidity = 0;
    int err = SimpleDHTErrSuccess;
    if ((err = dht11.read(&temperature, &humidity, NULL)) != SimpleDHTErrSuccess) {
        Serial.print("Read DHT11 failed, err=");
        Serial.print(SimpleDHTErrCode(err));
        Serial.print(",");
        Serial.println(SimpleDHTErrDuration(err));
        return;
    }
    Serial.print(lastFanState);
    Serial.print(", DHT OK: ");
    Serial.print((int) temperature);
    Serial.print("C, ");
    Serial.print((int) humidity);
    Serial.println("% H");


    if (lastFanState != FAN_OFF) {
        return;
    }

    if (humidity > HUMIDITY_XHIGH) {
        // shower on detected
        lastFanState = FAN_S2;
        updateFanAndStartTimer(lastFanState, FAN_DURATION_SHOWER_MS);
        Serial.println("Shower detected");
        return;
    }

    if (currentMillis < humidityAutoPauseUntil) {
        // short auto ventilation paused for 5 min
        return;
    }

    if (humidity > HUMIDITY_HIGH) {
        if (humidityHighCounter < 4) {
            humidityHighCounter++;
            return;
        }

        lastFanState = FAN_S1;
        // start short auto ventialion
        updateFanAndStartTimer(lastFanState, HUMIDITY_SHORT_AUTO_MS);
        // short auto ventilation paused for 10 min
        humidityAutoPauseUntil = currentMillis + HUMIDITY_AUTO_PAUSE_MS + HUMIDITY_SHORT_AUTO_MS;
        humidityHighCounter = 0;
        Serial.println("High humidity detected > short ventilation");
    }
}



