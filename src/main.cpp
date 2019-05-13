#include <Arduino.h>
#include <ArduinoJson.h>

#include "pusher.h"
#include "ledcStepper.h"

Pusher *_pusher;
unsigned int myTimer;

#define FORWARD 1
#define BACKWARDS 0
#define MSG_MOVE 1
#define MSG_SHOOT 2
#define MSG_CALIBRATE_START 3
#define MSG_CALIBRATE_FINISH 4
#define MSG_GOTO_POSITION 5
#define MSG_GOTO_ANGLE 6
#define MSG_MOVE_POSITION 7
#define MSG_MOVE_ANGLE 8

String commandLine;

//Flywheels
const int servoPin = 22;

const int panStepperStepPin = 14;
const int panStepperDirPin = 27;
const int panPWMChannel = 1;

const int tiltStepperStepPin = 13;
const int tiltStepperDirPin = 12;
const int tiltPWMChannel = 2;

DynamicJsonDocument doc(1024);


ledcStepper tiltStepper(2, tiltStepperDirPin, tiltStepperStepPin, 200, 32);
ledcStepper panStepper(4, panStepperDirPin, panStepperStepPin, 200, 32);


void tiltHwInterrupt()
{
  tiltStepper.hwInterrupt();
}

void panHwInterrupt()
{
  panStepper.hwInterrupt();
}

void setup()
{
  Serial.begin(115200);
  Serial.setTimeout(50);
  Serial.println("nerbot turret init");
  _pusher = new Pusher(servoPin, 2, 4, 33, 15);
  tiltStepper.stop();
  panStepper.stop();
  tiltStepper.attachInterrupt(&tiltHwInterrupt);
  panStepper.attachInterrupt(&panHwInterrupt);
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
        panStepper.freeRotate(directionX == FORWARD ? ledcStepper::RT_FORWARD : ledcStepper::RT_BACKWARDS, speedX);
      else
        panStepper.stop();

      if (speedY > 0)
        tiltStepper.freeRotate(directionY == FORWARD ? ledcStepper::RT_FORWARD : ledcStepper::RT_BACKWARDS, speedY);
      else
        tiltStepper.stop();

      break;
    }
    case MSG_GOTO_POSITION:
    {
      Serial.printf("position %ld %ld \r\n", panStepper.getPosition(), tiltStepper.getPosition());
      panStepper.goToPosition(positionX, speedX);
      tiltStepper.goToPosition(positionY, speedY);
      break;
    }
    case MSG_GOTO_ANGLE:
    {
      panStepper.goToAngle(angleX, speedX);
      tiltStepper.goToAngle(angleY, speedY);
      break;
    } 
    case MSG_MOVE_POSITION:
    {
      Serial.printf("position %ld %ld %ld \r\n", panStepper.getPosition(), tiltStepper.getPosition(), panStepper._stopCount);
      panStepper.movePosition(directionX == FORWARD ? ledcStepper::RT_FORWARD : ledcStepper::RT_BACKWARDS, positionX, speedX);
      tiltStepper.movePosition(directionY == FORWARD ? ledcStepper::RT_FORWARD : ledcStepper::RT_BACKWARDS, positionY, speedY);
      break;
    }   
    case MSG_MOVE_ANGLE:
    {
      panStepper.moveAngle(directionX == FORWARD ? ledcStepper::RT_FORWARD : ledcStepper::RT_BACKWARDS, angleX, speedX);
      tiltStepper.moveAngle(directionY == FORWARD ? ledcStepper::RT_FORWARD : ledcStepper::RT_BACKWARDS, angleY, speedY);
      break;
    }                 
    }
  }
}