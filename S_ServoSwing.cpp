//******************************************************************************************
//  S_ServoSwing.cpp
//  Author: Chris Whong
//  Based on S_TimedServo, this ST_Everything module is intended to swing a servo from
//  a start position, to an end position, and back to the start position.  This is useful
//  for making a physical button pusher
//
//  usage with ST_Everything as a relaySwitch:
//  static st::S_ServoSwing  sensor1(F("relaySwitch1"), controlPin, startAngle, endAngle, theDelay);
//
//******************************************************************************************

#include "S_ServoSwing.h"

#include "Constants.h"
#include "Everything.h"

namespace st
{

//private
void S_ServoSwing::cycleServo()
{
	if (!m_Servo.attached()) {
		m_Servo.attach(m_nPinPWM);
	}

	int pos = m_nStartAngle;
	for (pos = m_nStartAngle + 1; pos <= m_nEndAngle; pos += 1) {
		// in steps of 1 degree
		m_Servo.write(pos);              // tell servo to go to position in variable 'pos'
		delay(m_nDelay);                 // waits 15ms for the servo to reach the position
	}

	for (pos = m_nEndAngle - 1; pos >= m_nStartAngle; pos -= 1) {
		m_Servo.write(pos);              // tell servo to go to position in variable 'pos'
		delay(m_nDelay);                 // waits 15ms for the servo to reach the position
	}

	if (st::Executor::debug) {
		Serial.print(F("Swing Complete"));
	}

	m_bCurrentState = LOW;

	//Queue the relay status update the ST Cloud
	Everything::sendSmartString(getName() + " " + (m_bCurrentState == HIGH ? F("on") : F("off")));
}

//public
	//constructor
	S_ServoSwing::S_ServoSwing(const __FlashStringHelper *name, byte pinPWM, int startAngle, int endAngle, int delay) :
		Sensor(name),
		m_nPinPWM(pinPWM),			 //Arduino Pin used as a PWM Output for the switch level capability
		m_nStartAngle(startAngle),
		m_nEndAngle(endAngle),
		m_nDelay(delay)
		{
			setPWMPin(pinPWM);
		}

	//destructor
	S_ServoSwing::~S_ServoSwing()
	{
	}

	void S_ServoSwing::init()
	{
		// set the servo to the startAngle
		if (!m_Servo.attached()) {
			m_Servo.attach(m_nPinPWM);
		}

		m_Servo.write(m_nStartAngle);

		Everything::sendSmartString(getName() + " " + (m_bCurrentState == HIGH ? F("on") : F("off")));
	}

	//update function
	void S_ServoSwing::update()
	{

	}

	void S_ServoSwing::beSmart(const String &str)
	{
		String s = str.substring(str.indexOf(' ') + 1);
		if (st::Device::debug) {
			Serial.print(F("S_ServoSwing::beSmart s = "));
			Serial.println(s);
		}

		// if msg says turn on and currentState is off
		if ((s == F("on")) && (m_bCurrentState == LOW))
		{
			m_bCurrentState = HIGH;

			//Queue the relay status update the ST Cloud
			Everything::sendSmartString(getName() + " " + (m_bCurrentState == HIGH ? F("on") : F("off")));

			//update the digital output
			cycleServo();
		}
	}

	//called periodically by Everything class to ensure ST Cloud is kept consistent with the state of the contact sensor
	void S_ServoSwing::refresh()
	{
		//Queue the relay status update the ST Cloud
		Everything::sendSmartString(getName() + " " + (m_bCurrentState == HIGH ? F("on") : F("off")));
	}

	void S_ServoSwing::setPWMPin(byte pin)
	{
		m_nPinPWM = pin;
	}
}
