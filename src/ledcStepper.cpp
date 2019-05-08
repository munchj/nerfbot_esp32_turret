#include "ledcStepper.h"

ledcStepper::ledcStepper(uint8_t pwmChannel, uint8_t dirPin,  uint8_t stepPin, uint8_t stepsPerRevolution, uint8_t microStepping) {
    this->_pwmChannel = pwmChannel;
    this->_stepPin = stepPin;
    this->_dirPin = dirPin;
    this->_stepsPerRevolution = stepsPerRevolution;
    this->_microStepping = microStepping;

    //start the timer with a 0hz frequency, 1 duty cycle resolution 
    //drv8825 min pulse duration, both high and low is 1.8us
    //with 32 microstepping, 200 steps per revolution, 200rpm, we need 21333 steps/s 
    //which gives us about 47us (23.5us low/23.5us high) cycles
    ledcSetup(this->_pwmChannel, 0, 8);
    ledcAttachPin(this->_stepPin, this->_pwmChannel);
    ledcWrite(this->_pwmChannel, 4);

    //direction pin setup
    pinMode(this->_dirPin, OUTPUT);
    digitalWrite(this->_dirPin, HIGH);
}


ledcStepper::~ledcStepper() {

}

void ledcStepper::nextAction() {
    //nothing to do for now as everything is handled by the timer
}

void ledcStepper::setRPM(int rpm) {
    this->_rpm = rpm;
    this->_frequency = abs(this->_rpm) * this->_microStepping * this->_stepsPerRevolution / 60;
    digitalWrite(this->_dirPin, this->_rpm>=0?HIGH:LOW);
    ledcWriteTone(this->_pwmChannel, this->_frequency);
}

void ledcStepper::stop() {
    this->setRPM(0);
}

void ledcStepper::startRotate() {
    //this->setRPM(this->_rpm);
}