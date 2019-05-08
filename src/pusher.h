#include <Arduino.h>
#include <ESP32Servo.h>


#ifndef PUSHER_H
#define PUSHER_H

#define MOTOR_INERTIA_TIME 300

#define MIN_PULSE_LENGTH 1000
#define MAX_PULSE_LENGTH 2000

enum PusherState {
    INACTIVE,
    ENDSTOP_REACHED,
    RAMPING_UP,
    PUSHING,
    BRAKING,
    BRAKING_DONE
};

class Pusher {
    public:
        Pusher(uint8_t pwmChannel, uint8_t dirPin1,  uint8_t dirPin2,  uint8_t enablePin, uint8_t microSwitchPin);
        ~Pusher();

        void tick(); //do its things in a non blocking way
        void setSpeed(unsigned int speed);
        void push(unsigned int number_of_pushes = 30);
        void calibrate();
        void stop();

    private:
        uint8_t _servoPin;
        uint8_t _dirPin1;
        uint8_t _dirPin2;
        uint8_t _enablePin; //PWM
        uint8_t _microSwitchPin;
        PusherState _state;
        Servo _servo;

        unsigned int _speed;
        unsigned int _number_of_pushes_left;
        unsigned int _brakeTimer;

        void rotate();
        
        void brake(unsigned int duration);
        void setState(PusherState newState);
        

        //Delay
        long unsigned int _nextAction;
        void nonBlockingDelay(unsigned int duration);

};



#endif