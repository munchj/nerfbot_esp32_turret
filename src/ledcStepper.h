#include <Arduino.h>

#ifndef LEDC_STEPPER_H
#define LEDC_STEPPER_H

//TODO: use interrupts (needs 1 more pin) to track position of the stepper if needed
//TODO: acceleration?

class ledcStepper {
    public:
        enum DIRECTION {
            RT_FORWARD,
            RT_BACKWARDS
        };

        ledcStepper(uint8_t pwmChannel, uint8_t dirPin,  uint8_t stepPin, uint8_t stepsPerRevolution, uint8_t microStepping);
        ~ledcStepper();
        // optional: reduction ratio, default is 1
        void setReductionRatio(double reductionRatio);
        

        // movement methods
        void freeRotate(DIRECTION direction, double rpm);
        void goToPosition(long position, double rpm);
        void goToAngle(double angle, double rpm);
        void movePosition(DIRECTION direction, long position, double rpm);
        void moveAngle(DIRECTION direction, double angle, double rpm);
        void stop(); //immediate stop


        //homing, min/max
        void setHome();
        void setMinPos(long position);
        void setMaxPos(long position);
        void setMinAngle(double angle);
        void setMaxAngle(double angle);

        //utils
        long angleToTicks(double angle);
        double TicksToAngle(long ticks);


        ///// hardware interrupt to track position
        void setupInterrupt(void (*fn) ());
        void hwInterrupt();

        ///
        long getPosition();

    private:
        uint8_t _pwmChannel;
        uint8_t _timer;
        uint8_t _stepPin;
        uint8_t _dirPin;
        uint8_t _stepsPerRevolution;
        uint8_t _microStepping;
        double _reductionRatio;

        volatile long _position; //position in ticks
        bool _enableMinPosition;
        long _minPosition;
        bool _enableMaxPosition;
        long _maxPosition;
        long _wantedPosition;
        bool _freeRotation;

        //mutex
        portMUX_TYPE _timerMux;
        hw_timer_t * _timer_t;

        unsigned long _frequency;
        DIRECTION _direction;
        
        /////
        void _rotate();
        void _setRPM(double rpm);
        void _setFrequency(unsigned int frequency);



};



#endif