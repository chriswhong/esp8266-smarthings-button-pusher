//******************************************************************************************
//  S_ServoSwing.h
//  Author: Chris Whong
//  Based on S_TimedServo, this ST_Everything module is intended to swing a servo from
//  a start position, to an end position, and back to the start position.  This is useful
//  for making a physical button pusher
//
//  usage with ST_Everything as a relaySwitch:
//  static st::S_ServoSwing  sensor1(F("relaySwitch1"), controlPin, startAngle, endAngle, theDelay);
//
//******************************************************************************************

#ifndef ST_S_SERVOSWING_H
#define ST_S_SERVOSWING_H

#include "Sensor.h"
#include <Servo.h>

namespace st
{
	class S_ServoSwing : public Sensor  //inherits from parent Sensor Class
	{
		private:

			//following are for the digital output
			Servo m_Servo;
			byte m_nPinPWM;			 //Arduino Pin used as a PWM Output for the switch level capability
			int m_nStartAngle;
			int m_nEndAngle;
			int m_nDelay;
			bool m_bCurrentState;	        //HIGH or LOW
			void cycleServo();	//function to update the Arduino PWM Output Pin

		public:
			//constructor - called in your sketch's global variable declaration section
			S_ServoSwing(const __FlashStringHelper *name, byte pinPWM, int startAngle = 0, int endAngle = 90, int delay = 15);

			//destructor
			virtual ~S_ServoSwing();

			//initialization function
			virtual void init();

			//update function
			void update();

			//SmartThings Shield data handler (receives command to turn "on" or "off" the switch (digital output)
			virtual void beSmart(const String &str);

			//called periodically by Everything class to ensure ST Cloud is kept consistent with the state of the contact sensor
			virtual void refresh();

			//gets
			virtual byte getPin() const { return m_nPinPWM; }

			//sets
			virtual void setPWMPin(byte pin);
	};
}


#endif
