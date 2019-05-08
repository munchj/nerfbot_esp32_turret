#include "pusher.h"

char *PusherStateName[] =
    {
        "INACTIVE",
        "ENDSTOP_REACHED",
        "RAMPING_UP",
        "PUSHING",
        "BRAKING",
        "BRAKING_DONE"};

Pusher::Pusher(uint8_t servoPin, uint8_t dirPin1, uint8_t dirPin2, uint8_t enablePin, uint8_t microSwitchPin)
{

    Serial.printf("new Pusher %d %d %d %d %d", servoPin, dirPin1, dirPin2, enablePin, microSwitchPin);
    Serial.println("");
    this->_servoPin = servoPin;
    this->_dirPin1 = dirPin1;
    this->_dirPin2 = dirPin2;
    this->_enablePin = enablePin;
    this->_microSwitchPin = microSwitchPin;
    this->_number_of_pushes_left = 0;
    this->_speed = 18;

    pinMode(this->_dirPin1, OUTPUT);
    pinMode(this->_dirPin2, OUTPUT);
    pinMode(this->_enablePin, OUTPUT);
    pinMode(this->_microSwitchPin, INPUT);

    this->_servo.attach(servoPin, MIN_PULSE_LENGTH, MAX_PULSE_LENGTH);
    this->_servo.writeMicroseconds(MAX_PULSE_LENGTH);
}

Pusher::~Pusher()
{
}

void Pusher::setState(PusherState newState)
{
    //Serial.printf("Changing state from %s to %s [%lu]\n", PusherStateName[_state], PusherStateName[newState], millis());
    _state = newState;
}

void Pusher::nonBlockingDelay(unsigned int duration)
{
    // non blocking delay
    this->_nextAction = millis() + duration;
}

void Pusher::tick()
{
    long unsigned int currentTime = millis();
    if (_nextAction <= currentTime)
    {
        unsigned int microSwitchTriggered = digitalRead(this->_microSwitchPin);
        switch (_state)
        {
        case RAMPING_UP:
        {
            this->setState(ENDSTOP_REACHED);
            digitalWrite(this->_dirPin1, HIGH);
            digitalWrite(this->_dirPin2, LOW);
            nonBlockingDelay(30);
            break;
        }
        case BRAKING:
        {
            this->setState(BRAKING_DONE);
            digitalWrite(this->_dirPin1, LOW);
            digitalWrite(this->_dirPin2, LOW);
            break;
        }
        case ENDSTOP_REACHED:
        {
            if (!microSwitchTriggered)
            {
                //the pusher moved away from the microswitch
                this->setState(PUSHING);
                nonBlockingDelay(30);
            }
            break;
        }
        case PUSHING:
        {
            if (microSwitchTriggered && this->_number_of_pushes_left > 0)
            {
                this->_state = ENDSTOP_REACHED;
                this->_number_of_pushes_left--;
                Serial.printf("microswitch triggered [%u left] [%lu]\n", this->_number_of_pushes_left, millis());

                nonBlockingDelay(30);
            }

            if (_number_of_pushes_left == 0)
            {
                this->stop();
            }
            else
            {
            }
            break;
        }
        default:
        {
        }
        }
    }
    else
    {
        //waiting for next action
    }
}

void Pusher::setSpeed(unsigned int speed)
{
    this->_speed = speed;
}

void Pusher::push(unsigned int number_of_pushes)
{
    Serial.printf("Pusher::push %d\n", number_of_pushes);
    if (this->_number_of_pushes_left == 0)
    {
        this->_number_of_pushes_left = number_of_pushes;
        this->rotate();
    }
    else {
        this->_number_of_pushes_left += number_of_pushes;
    }
}

void Pusher::stop()
{
    this->setState(INACTIVE);
    this->brake(25);
    this->_servo.writeMicroseconds(MIN_PULSE_LENGTH);
}

void Pusher::calibrate()
{
    this->_servo.writeMicroseconds(MIN_PULSE_LENGTH);
    nonBlockingDelay(5000);
}

void Pusher::rotate()
{
    this->_servo.writeMicroseconds(1000 + (10 * this->_speed));
    this->setState(RAMPING_UP);
    nonBlockingDelay(300);
}

void Pusher::brake(unsigned int duration)
{
    this->setState(BRAKING);
    digitalWrite(this->_dirPin1, LOW);
    digitalWrite(this->_dirPin2, HIGH);
    nonBlockingDelay(duration);
}