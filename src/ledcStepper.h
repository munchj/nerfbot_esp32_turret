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
        void freeRotateF(DIRECTION direction, unsigned int frequency);

        void goToPosition(long position, double rpm);
        void goToPositionF(long position, unsigned int frequency); 

        void goToAngle(long angle, double rpm);
        void goToAngleF(long angle, unsigned int frequency);               
        
        void movePosition(DIRECTION direction, long position, double rpm);
        void movePositionF(DIRECTION direction, long position, unsigned int frequency);

        void moveAngle(DIRECTION direction, long angle, double rpm);
        void moveAngleF(DIRECTION direction, long angle, unsigned int frequency);        

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
        void attachInterrupt(void (*fn) ());
        void hwInterrupt();

    private:
        uint8_t _pwmChannel;
        uint8_t _timer;
        uint8_t _stepPin;
        uint8_t _dirPin;
        uint8_t _stepsPerRevolution;
        uint8_t _microStepping;
        double _reductionRatio;

        long _position; //position in ticks
        long _minPosition;
        long _maxPosition;
        long _wantedPosition;
        bool _freeRotation;


        unsigned long _frequency;
        DIRECTION _direction;
        
        /////
        void _rotate();
        void _setRPM(double rpm);
        void _setFrequency(unsigned int frequency);



};



#endif