//
// Created by thomas on 24.09.2022.
//

#ifndef CIRCULATIONCONTROL_CIRCULATIONCONTROLER_H
#define CIRCULATIONCONTROL_CIRCULATIONCONTROLER_H

#include <Arduino.h>

// circulation configuration
#define OPEN LOW
#define CLOSE HIGH
#define PUMP_ON LOW
#define PUMP_OFF HIGH
#define BUTTON_PRESSED LOW
#define BUTTON_RELEASED HIGH

#define W1_B_VALVE 2
#define W1_K_VALVE 3
#define W1_G_VALVE 4
#define W2_X_VALVE 5
#define W3_B_VALVE 6
#define W3_k_VALVE 6

#define W1_B_BUTTON 10
#define W1_K_BUTTON 11
#define W1_G_BUTTON 12
#define W2_X_BUTTON 13
#define W3_B_BUTTON 17
#define W3_K_BUTTON 16

#define CIRCULATION_PUMP_PIN 7

struct Tap {
    // name of tap
    const String name;
    // pin of button
    const int button;
    // pin of valve relais
    const int valve;
    // time in seconds circulation pump works for this tap. need to be adjusted when installed
    const int circulationTime;
    // timestamp of next stop of pump or 0 when inactive
    //unsigned long timeout;
};


class CirculationControler {
public:
    static void initCirculationController();
    void handleTaps();
};


#endif //CIRCULATIONCONTROL_CIRCULATIONCONTROLER_H
