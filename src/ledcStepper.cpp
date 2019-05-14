#include "ledcStepper.h"
#include "esp32-hal-timer.h"

void ledcStepper::hwInterrupt()
{
    portENTER_CRITICAL_ISR(&this->_timerMux);
    //Serial.println("interrupt");
    if (this->_direction == RT_FORWARD) this->_position++;
    else this->_position--;

    if(!this->_freeRotation) {
        if(this->_wantedPosition == this->_position) {
            
            this->stop();
        }
    }

    if(this->_enableMinPosition && this->_position <= this->_minPosition) stop();
    if(this->_enableMaxPosition && this->_position >= this->_maxPosition) stop();
    portEXIT_CRITICAL_ISR(&this->_timerMux);

}

void ledcStepper::setupInterrupt(void (*fn)())
{
    //start the timer with a 0hz frequency, 1 duty cycle resolution
    //drv8825 min pulse duration, both high and low is 1.8us
    //with 32 microstepping, 200 steps per revolution, 200rpm, we need 21333 steps/s
    //which gives us about 47us (23.5us low/23.5us high) cycles
    ledcSetup(this->_pwmChannel, 0, 8);
    ledcAttachPin(this->_stepPin, this->_pwmChannel);
    ledcWrite(this->_pwmChannel, 4);

    attachInterrupt(digitalPinToInterrupt(this->_stepPin), fn, FALLING);
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
    this->_enableMinPosition = false;
    this->_enableMaxPosition = false;
    this->_position = 0;

    this->_timerMux = portMUX_INITIALIZER_UNLOCKED;
    


    //direction pin setup
    pinMode(this->_dirPin, OUTPUT);
    digitalWrite(this->_dirPin, HIGH);
}

ledcStepper::~ledcStepper()
{
}

long ledcStepper::getPosition() {
    return this->_position;
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
    portENTER_CRITICAL(&this->_timerMux);
    this->_setRPM(0);
    this->_rotate();
    portEXIT_CRITICAL(&this->_timerMux);
}

long ledcStepper::angleToTicks(double angle) {
    return this->_stepsPerRevolution * this->_microStepping * this->_reductionRatio * angle / 360.0;
}

double ledcStepper::TicksToAngle(long ticks) {
    return ticks * 360.0 / (this->_stepsPerRevolution * this->_microStepping * this->_reductionRatio);
}

void ledcStepper::setHome() {
    portENTER_CRITICAL(&this->_timerMux);
    this->_position = 0;
    portEXIT_CRITICAL(&this->_timerMux);
}

void ledcStepper::setMinPos(long position) {
    this->_enableMinPosition = true;
    this->_minPosition = position;
}

void ledcStepper::setMaxPos(long position) {
    this->_enableMaxPosition = true;
    this->_maxPosition = position;
}

void ledcStepper::setMinAngle(double angle) {
    this->_enableMinPosition = true;
    this->_minPosition = angleToTicks(angle);
}

void ledcStepper::setMaxAngle(double angle) {
    this->_enableMaxPosition = true;
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
    Serial.printf("goToPosition %ld %f \r\n", position, rpm);
    portENTER_CRITICAL(&this->_timerMux);
    this->_freeRotation = false;
    this->_setRPM(rpm);
    
    if (this->_position == position) {
        portEXIT_CRITICAL(&this->_timerMux);
        return; 
    } 
    
    this->_wantedPosition = position;

    this->_direction =  (position > this->_position) ? RT_FORWARD : RT_BACKWARDS;

    this->_rotate();
    portEXIT_CRITICAL(&this->_timerMux);
}

void ledcStepper::goToAngle(long angle, double rpm) {
    Serial.printf("goToAngle %ld %f \r\n", angle, rpm);
    goToPosition(angleToTicks(angle), rpm);
}


void ledcStepper::movePosition(DIRECTION direction, long position, double rpm) {
     Serial.printf("goToAngle %d %ld %f \r\n",direction, position, rpm);
    portENTER_CRITICAL(&this->_timerMux);
    this->_freeRotation = false;
    if(direction == RT_FORWARD) {
        goToPosition(this->_position + position, rpm);
    }
    else {
        goToPosition(this->_position - position, rpm);
    }
    portEXIT_CRITICAL(&this->_timerMux);
}


void ledcStepper::moveAngle(DIRECTION direction, long angle, double rpm) {
    Serial.printf("moveAngle %d %ld %f \r\n",direction, angle, rpm);
    this->_freeRotation = false;
    movePosition(direction, angleToTicks(angle), rpm);
}



void ledcStepper::movePositionF(DIRECTION direction, long position, unsigned int frequency) {
    portENTER_CRITICAL(&this->_timerMux);
    this->_freeRotation = false;
    if(direction == RT_FORWARD) {
        goToPositionF(this->_position + position, frequency);
    }
    else {
        goToPositionF(this->_position - position, frequency);
    }
    portEXIT_CRITICAL(&this->_timerMux);
}



void ledcStepper::moveAngleF(DIRECTION direction, long angle, unsigned int frequency) {
    this->_freeRotation = false;
    movePositionF(direction, angleToTicks(angle), frequency);
}     


void ledcStepper::freeRotateF(DIRECTION direction, unsigned int frequency)
{
    this->_freeRotation = false;
    this->_direction = direction;
    this->_setFrequency(frequency);
    this->_freeRotation = true;
    this->_rotate();
}

void ledcStepper::goToPositionF(long position, unsigned int frequency)
{
    portENTER_CRITICAL(&this->_timerMux);
    this->_freeRotation = false;
    this->_setFrequency(frequency);

       if (this->_position == position) {
        portEXIT_CRITICAL(&this->_timerMux);
        return; 
    } 

    this->_wantedPosition = position;
    this->_direction =  (position > this->_position) ? RT_FORWARD : RT_BACKWARDS;

    this->_rotate(); 
    portEXIT_CRITICAL(&this->_timerMux);
}

void ledcStepper::goToAngleF(long angle, unsigned int frequency) {
    goToPositionF(angleToTicks(angle), frequency);
}
