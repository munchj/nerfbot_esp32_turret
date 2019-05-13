#include "ledcStepper.h"
#include "esp32-hal-timer.h"

void ledcStepper::hwInterrupt()
{
    if (this->_direction == RT_FORWARD) this->_position++;
    else this->_position--;

    if(!this->_freeRotation) {
        if(this->_wantedPosition == this->_position) {
            this->stop();
        }
    }

    if(this->_position <= this->_minPosition) stop();
    if(this->_position >= this->_maxPosition) stop();


}

void ledcStepper::attachInterrupt(void (*fn)())
{
    hw_timer_t *_timer_t = timerGet(this->_timer);
    timerAttachInterrupt(_timer_t, fn, true);
}


ledcStepper::ledcStepper(uint8_t pwmChannel, uint8_t dirPin, uint8_t stepPin, uint8_t stepsPerRevolution, uint8_t microStepping)
{
    this->_pwmChannel = pwmChannel;
    this->_timer = ((pwmChannel / 2) % 4);
    this->_stepPin = stepPin;
    this->_dirPin = dirPin;
    this->_stepsPerRevolution = stepsPerRevolution;
    this->_microStepping = microStepping;
    this->_reductionRatio = 1;

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

ledcStepper::~ledcStepper()
{
}

void ledcStepper::setReductionRatio(double reductionRatio) {
    this->_reductionRatio = reductionRatio;
}

void ledcStepper::_setRPM(double rpm)
{
    this->_frequency = rpm * this->_microStepping * this->_stepsPerRevolution * this->_reductionRatio / 60.0;
}

void ledcStepper::_setFrequency(unsigned int frequency)
{
    this->_frequency = frequency;
}

void ledcStepper::stop()
{
    this->_setRPM(0);
    this->_rotate();
}

long ledcStepper::angleToTicks(double angle) {
    return this->_stepsPerRevolution * this->_microStepping * this->_reductionRatio * angle / 360.0;
}

double ledcStepper::TicksToAngle(long ticks) {
    return ticks * 360.0 / (this->_stepsPerRevolution * this->_microStepping * this->_reductionRatio);
}

void ledcStepper::setHome() {
    this->_position = 0;
}

void ledcStepper::setMinPos(long position) {
    this->_minPosition = position;
}

void ledcStepper::setMaxPos(long position) {
    this->_maxPosition = position;
}

void ledcStepper::setMinAngle(double angle) {
    this->_minPosition = angleToTicks(angle);
}

void ledcStepper::setMaxAngle(double angle) {
    this->_maxPosition = angleToTicks(angle);
}

void ledcStepper::_rotate()
{
    digitalWrite(this->_dirPin, this->_direction == RT_FORWARD ? HIGH : LOW);
    ledcWriteTone(this->_pwmChannel, this->_frequency);
}

void ledcStepper::freeRotate(DIRECTION direction, double rpm)
{
    this->_direction = direction;
    this->_setRPM(rpm);
    this->_freeRotation = true;
    this->_rotate();
}

void ledcStepper::goToPosition(long position, double rpm)
{
    this->_freeRotation = false;
    this->_setRPM(rpm);

    if (this->_position == position) return; //

    this->_position = position;
    this->_direction =  (position > this->_position) ? RT_FORWARD : RT_BACKWARDS;

    this->_rotate();
  
}

void ledcStepper::goToAngle(long angle, double rpm) {
    goToPosition(angleToTicks(angle), rpm);
}

void ledcStepper::freeRotateF(DIRECTION direction, unsigned int frequency)
{
    this->_direction = direction;
    this->_setFrequency(frequency);
    this->_freeRotation = true;
    this->_rotate();
}

void ledcStepper::goToPositionF(long position, unsigned int frequency)
{
    this->_freeRotation = false;
    this->_setFrequency(frequency);

    if (this->_position == position) return; //

    this->_position = position;
    this->_direction =  (position > this->_position) ? RT_FORWARD : RT_BACKWARDS;

    this->_rotate(); 
}

void ledcStepper::goToAngleF(long angle, unsigned int frequency) {
    goToPositionF(angleToTicks(angle), frequency);
}


void ledcStepper::movePosition(DIRECTION direction, long position, double rpm) {
    if(direction == RT_FORWARD) {
        goToPosition(this->_position + position, rpm);
    }
    else {
        goToPosition(this->_position - position, rpm);
    }
}

void ledcStepper::movePositionF(DIRECTION direction, long position, unsigned int frequency) {
    if(direction == RT_FORWARD) {
        goToPositionF(this->_position + position, frequency);
    }
    else {
        goToPositionF(this->_position - position, frequency);
    }
}

void ledcStepper::moveAngle(DIRECTION direction, long angle, double rpm) {
    movePosition(direction, angleToTicks(angle), rpm);
}

void ledcStepper::moveAngleF(DIRECTION direction, long angle, unsigned int frequency) {
    movePositionF(direction, angleToTicks(angle), frequency);
}     