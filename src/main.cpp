#include <Arduino.h>
#include <ArduinoJson.h>


#include <DRV8825.h>
#include "pusher.h"
#include "ledcStepper.h"

Pusher *_pusher;
unsigned int myTimer;



#define FORWARD 1
#define BACKWARDS 0
#define MSG_MOVE 1
#define MSG_SHOOT 2
#define MSG_CALIBRATE 3


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

//DRV8825 tiltStepper(32*200, tiltStepperDirPin, tiltStepperStepPin);
//DRV8825 panStepper(32*200, panStepperDirPin, panStepperStepPin);
ledcStepper tiltStepper(1, tiltStepperDirPin, tiltStepperStepPin, 200, 32);
ledcStepper panStepper(2, panStepperDirPin, panStepperStepPin, 200, 32);


void setup()
{

  Serial.begin(115200);
  Serial.setTimeout(50);
  Serial.println("nerbot turret init");
  _pusher = new Pusher(servoPin, 2, 4, 33, 15);

}

void loop()
{
  _pusher->tick();



  if (Serial.available())
  {
    deserializeJson(doc, Serial);
    long type = doc["type"];
    long speedX = doc["speedX"];
    long speedY = doc["speedY"];
    long directionX = doc["directionX"];
    long directionY = doc["directionY"];

    switch (type)
    {
    case MSG_SHOOT:
    {
      if(speedX > 0) {
        _pusher->setSpeed(speedX);
      }
      else {
        speedX = 18;
      }
      
      _pusher->push(1);
      break;
    }
    case MSG_CALIBRATE:
    {
      _pusher->calibrate();
      break;
    }    
    case MSG_MOVE:
    {
      if (speedX > 0)
      {
        //panStepper.stop();
        panStepper.setRPM(directionX == FORWARD?speedX:-speedX);
        //panStepper.startRotate((directionX == FORWARD ? 100 : -100) * 360);
        
      }
      else
      {
        panStepper.stop();
      }

      if (speedY > 0)
      {
        //tiltStepper.stop();
        tiltStepper.setRPM(directionY == FORWARD ?speedY:-speedY);
        //tiltStepper.startRotate((directionY == FORWARD ? 100 : -100) * 360);
      }
      else
      {
        tiltStepper.stop();
      }
      break;
    }
    }
  }

  panStepper.nextAction();
  tiltStepper.nextAction();
}