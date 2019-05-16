#include <Arduino.h>
#include <ArduinoJson.h>

#include "helper.h"
#include "pusher.h"
#include "ledcStepper.h"


Pusher *_pusher;
unsigned int myTimer;



String commandLine;

//Flywheels
const int servoPin = 22;

const int tiltStepperStepPin = 14;
const int tiltStepperDirPin = 27;
const int tiltPWMChannel = 2;

const int panStepperStepPin = 13;
const int panStepperDirPin = 12;
const int panPWMChannel = 4;

DynamicJsonDocument doc(1024);


ledcStepper * _tiltStepper;
ledcStepper * _panStepper;


void tiltHwInterrupt()
{
  _tiltStepper->hwInterrupt();
}

void panHwInterrupt()
{
  _panStepper->hwInterrupt();
}

void setup()
{
  Serial.begin(115200);
  Serial.setTimeout(50);
  Serial.println("nerbot turret init");
  _pusher = new Pusher(servoPin, 2, 4, 33, 15);
  _tiltStepper = new ledcStepper(tiltPWMChannel, tiltStepperDirPin, tiltStepperStepPin, 200, 32);
  _panStepper = new ledcStepper(panPWMChannel, panStepperDirPin, panStepperStepPin, 200, 32);
  _tiltStepper->setupInterrupt(&tiltHwInterrupt);
  _panStepper->setupInterrupt(&panHwInterrupt);
  _tiltStepper->stop();
  _panStepper->stop();  
  _tiltStepper->setReductionRatio(5);
  _panStepper->setReductionRatio(6.66666666);
}
 
void loop()
{
  _pusher->tick();

  if (Serial.available())
  {
    deserializeJson(doc, Serial);
    long type = doc["type"];
    double speedX = doc["speedX"];
    double speedY = doc["speedY"];
    long directionX = doc["directionX"];
    long directionY = doc["directionY"];
    long positionX = doc["positionX"];
    long positionY = doc["positionY"];
    double angleX = doc["angleX"];
    double angleY = doc["angleY"];

    

    switch (type)
    {
    case MSG_SHOOT:
    {
      if (speedX > 0)
      {
        _pusher->setSpeed(speedX);
      }
      else
      {
        speedX = 18;
      }

      _pusher->push(1);
      break;
    }
    case MSG_CALIBRATE_START:
    {
      _pusher->calibrateStart();
      break;
    }        
    case MSG_CALIBRATE_FINISH:
    {
      _pusher->calibrateFinish();
      break;
    }
    case MSG_MOVE:
    {
      if (speedX > 0)
        _panStepper->freeRotate(static_cast<DIRECTION>(directionX), speedX);
      else
        _panStepper->stop();

      if (speedY > 0)
        _tiltStepper->freeRotate(static_cast<DIRECTION>(directionY), speedY);
      else
        _tiltStepper->stop();

      break;
    }
    case MSG_GOTO_POSITION:
    {
      _panStepper->goToPosition(positionX, speedX);
      _tiltStepper->goToPosition(positionY, speedY);
      break;
    }
    case MSG_GOTO_ANGLE:
    {
      _panStepper->goToAngle(angleX, speedX);
      _tiltStepper->goToAngle(angleY, speedY);
      break;
    } 
    case MSG_MOVE_POSITION:
    {
      _panStepper->movePosition(static_cast<DIRECTION>(directionX), positionX, speedX);
      _tiltStepper->movePosition(static_cast<DIRECTION>(directionY), positionY, speedY);
      break;
    }   
    case MSG_MOVE_ANGLE:
    {
      _panStepper->moveAngle(static_cast<DIRECTION>(directionX), angleX, speedX);
      _tiltStepper->moveAngle(static_cast<DIRECTION>(directionY), angleY, speedY);
      break;
    }   
    case MSG_HOME:
    {
      _panStepper->setHome();
      _tiltStepper->setHome();
      break;
    }                     
    }
  }
}