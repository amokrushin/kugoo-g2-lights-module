#include <Arduino.h>
#include <avr/wdt.h>
#include <TaskScheduler.h>                  // Scheduler
#include <AH/Timing/MillisMicrosTimer.hpp>  // Timer
#include <Filters/MedianFilter.hpp>         // MedianFilter


const int BRAKE_THROTTLE_IN_PIN = 14;       // PC0
const int ACCELERATOR_THROTTLE_IN_PIN = 15; // PC1
const int HEAD_LIGHT_SWITCH_IN_PIN = 16;    // PC2
const int HORN_SWITCH_IN_PIN = 17;          // PC3

const int SIDE_LIGHT_OUT_PIN = 8;           // PB0
const int STOP_LIGHT_OUT_PIN = 9;           // PB1
const int HORN_OUT_PIN = 10;                // PB2
const int HEAD_LIGHT_OUT_PIN = 11;          // PB3


// Sample timer
AH::Timer <millis> timer = 5;
// Median filter of length 10, initialized with a value of 0.
MedianFilter<10, uint16_t> brakeFilter = {0};
MedianFilter<10, uint16_t> acceleratorFilter = {0};
MedianFilter<10, uint16_t> headLightFilter = {0};
MedianFilter<10, uint16_t> hornFilter = {0};
// Scheduler
Scheduler runner;


void mainHandler() {
    int acceleratorThrottleValue = acceleratorFilter(analogRead(ACCELERATOR_THROTTLE_IN_PIN));
    int brakeThrottleValue = brakeFilter(analogRead(BRAKE_THROTTLE_IN_PIN));
    int headLightValue = headLightFilter(digitalRead(HEAD_LIGHT_SWITCH_IN_PIN) * 100);
    int hornValue = hornFilter(digitalRead(HORN_SWITCH_IN_PIN) * 100);

    if (timer) {
        if ((acceleratorThrottleValue < 10 || brakeThrottleValue > 300)) {
            digitalWrite(STOP_LIGHT_OUT_PIN, HIGH);
        } else {
            digitalWrite(STOP_LIGHT_OUT_PIN, LOW);
        }

        if (brakeThrottleValue > 300) {
            pinMode(ACCELERATOR_THROTTLE_IN_PIN, OUTPUT);
            digitalWrite(ACCELERATOR_THROTTLE_IN_PIN, LOW);
        } else {
            pinMode(ACCELERATOR_THROTTLE_IN_PIN, INPUT);
        }

        if (headLightValue > 50) {
            digitalWrite(HEAD_LIGHT_OUT_PIN, HIGH);
            digitalWrite(SIDE_LIGHT_OUT_PIN, HIGH);
        } else {
            digitalWrite(HEAD_LIGHT_OUT_PIN, LOW);
            digitalWrite(SIDE_LIGHT_OUT_PIN, LOW);
        }

        if (hornValue > 50) {
            digitalWrite(HORN_OUT_PIN, HIGH);
        } else {
            digitalWrite(HORN_OUT_PIN, LOW);
        }
    }
}

void heartbeatHandler() {
    wdt_reset();

    if (digitalRead(LED_BUILTIN) == HIGH) {
        digitalWrite(LED_BUILTIN, LOW);
    } else {
        digitalWrite(LED_BUILTIN, HIGH);
    }
}

//Tasks
Task mainTask(TASK_IMMEDIATE, TASK_FOREVER, &mainHandler);
Task heartbeatTask(100, TASK_FOREVER, &heartbeatHandler);

void setup() {
    wdt_enable(WDTO_250MS);

    pinMode(ACCELERATOR_THROTTLE_IN_PIN, INPUT);
    pinMode(BRAKE_THROTTLE_IN_PIN, INPUT);
    pinMode(HEAD_LIGHT_SWITCH_IN_PIN, INPUT);
    pinMode(HORN_SWITCH_IN_PIN, INPUT);

    pinMode(SIDE_LIGHT_OUT_PIN, OUTPUT);
    pinMode(STOP_LIGHT_OUT_PIN, OUTPUT);
    pinMode(HEAD_LIGHT_OUT_PIN, OUTPUT);
    pinMode(HORN_OUT_PIN, OUTPUT);

    pinMode(LED_BUILTIN, OUTPUT);

    runner.init();
    runner.addTask(mainTask);
    runner.addTask(heartbeatTask);
    mainTask.enable();
    heartbeatTask.enable();
}

void loop() {
    runner.execute();
}