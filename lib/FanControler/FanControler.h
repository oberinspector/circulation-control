//
// Created by thomas on 24.09.2022.
//

#ifndef CIRCULATIONCONTROL_FANCONTROLER_H
#define CIRCULATIONCONTROL_FANCONTROLER_H

#include <arduino.h>
#include <SimpleDHT.h>

#define BUTTON_STATE_PRESSED true
#define BUTTON_STATE_RELEASED false

#define RELAYS_OPEN = HIGH
#define RELAYS_CLOSED = LOW

const int BATH_FAN_SPEED_RELAYS_PIN = 8; // default fan speed 1.
const int BATH_FAN_POWER_RELAYS_PIN = 9; // switch on fan
const int BATH_FAN_BUTTON = A1;

// button detect times in ms
const int FAN_BUTTON_TRESHHOLD_MS = 20;
const int FAN_BUTTON_SHORT_MS = 250;
const int FAN_BUTTON_LONG_MS = 1500; // increase timer
const int FAN_BUTTON_XLONG_MS = 5000; // switch fan auto (DHT) mode on or off

const unsigned long HUMIDITY_CHECK_INTERVALL = 10000;
//
const unsigned long HUMIDITY_AUTO_PAUSE_MS = 600000;
const unsigned long HUMIDITY_SHORT_AUTO_MS = 120000;
const unsigned long FAN_DURATION_MS = 300000;
const unsigned long FAN_DURATION_SHOWER_MS = 300000;

const byte HUMIDITY_XHIGH = 93;
const byte HUMIDITY_HIGH = 86;
const byte TEMP_SHOWER_ON = 22;

// for DHT11,
//      VCC: 5V or 3V
//      GND: GND
//      DATA: 2
const int DHT11_DATA_PIN = A0;

enum FanState {
    FAN_OFF, FAN_S1, FAN_S2
};
enum FanButtonEvent {
    NONE, FAN_BUTTON_SHORT, FAN_BUTTON, FAN_BUTTON_LONG, FAN_BUTTON_XLONG
};


class FanControler {
public:
    void initFanController();

    void handleFanControler();


    FanButtonEvent checkButton(unsigned long currentMillis);

    void handleFanButtonEvent(unsigned long millis);

    FanState rotateFanState();

    void updateFanAndStartTimer(FanState state, unsigned long duration);

    void increaseTimer();

    void handleHumidityFanControl(unsigned long currentMillis);
};


#endif //CIRCULATIONCONTROL_FANCONTROLER_H
