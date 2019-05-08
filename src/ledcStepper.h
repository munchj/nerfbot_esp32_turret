#include <Arduino.h>

#ifndef LEDC_STEPPER_H
#define LEDC_STEPPER_H

//TODO: use interrupts (needs 1 more pin) to track position of the stepper if needed
//TODO: acceleration?

class ledcStepper {
    public:
        ledcStepper(uint8_t pwmChannel, uint8_t dirPin,  uint8_t stepPin, uint8_t stepsPerRevolution, uint8_t microStepping);
        ~ledcStepper();

        void nextAction(); //do its things in a non blocking way
        void setRPM(int rpm);
        void stop();
        void startRotate();

    private:
        uint8_t _pwmChannel;
        uint8_t _stepPin;
        uint8_t _dirPin;
        uint8_t _stepsPerRevolution;
        uint8_t _microStepping;

        int _rpm;
        unsigned long _frequency;

};



#endif