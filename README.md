# esp8266-smarthings-button-pusher

Code for creating a smartthings-compatible servo-based button-pusher from a nodeMCU esp8266 wifi-enabled arduino-compatiable microcontroller.

## Background

I had an existing smartthings network at home, with mostly z-wave devices such as light switches.  The smartthings-Alexa integration makes it dirt simple to add voice commands to anything that's already on the smartthings network, such as "Alexa, turn off the living room lights".

I wanted the same voice control for a physical button that needed to be pressed in my home.  This youtube video, [How to Make an Alexa Controlled Finger](https://www.youtube.com/watch?v=afPxj0LS0LU) caught my eye and introduced me to the esp8266, a low-cost arduino-compatible microcontroller with built-in wifi. In the video, the maker does not use a smarthome bridge, but instead uses a library that will make the esp8266 emulate an off-the-shelf WEMO smart switch.  I started to go down this route and bought some esp8266s and a servo, but found that the library in question didn't work with all models/generations of Alexa, and was not going to be as straightforward as the video made it seem.

Further research pointed me to the [ST_Anything](https://github.com/DanielOgorchock/ST_Anything) library, which contains lots of examples and device libraries for turning an esp8266 into a Smartthings device, be it a switch, servo controller, thermistor, button, door control, etc.  ST_Anything has attempted to make everything as simple as possible, meaning all you need to do is enter wifi credentials and ip addresses, etc, to get up and running quickly.

ST_Anything consists of two basic parts, the arduino code samples/libraries, and the smartthings device handlers.  I was able to get both sides installed and talking to each other quickly thanks to lots of examples and forum threads.  However, there was no ready to go button-pusher library that would suit my needs.

## The Arduino Side

There was one ST_Anything library that came close, `TimedRelay`.  To smartthings, it is a switch, but the arduino code receives and `on` command, activates a digital output, then sends and `off` command back to the smartthings hub, effectively simulating a button push.  This was exactly the same behavior I wanted for a button pusher, so what I needed to do was modify the `TimedRelay` library to swing a servo instead of activate a digital output pin.

The result is `S_ServoSwing`, a new ST_Anything library I put together based on `TimedRelay` and some basic arduino servo code.  

In my main arduino sketch, I can now use one line of code to tell `S_ServoSwing` which pin the servo is on, what the start and end angle should be, and how long in milliseconds to wait between each degree of movement.

```
const unsigned int startAngle = 50;
const unsigned int endAngle = 180;
const unsigned int theDelay = 10;

// delcare the servo
static st::S_ServoSwing          sensor1(F("relaySwitch"), PIN_SERVO, startAngle, endAngle, theDelay);
```

The `S_ServoSwing` library controls the servo, using a couple of for loops to go from the startAngle, to the endAngle, and back again:

```
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
```

That last line sends a command back to the smartthings hub that changes the switch state to `off` after the servo movement is complete.

## The Smartthings Side

ST_Anything is pretty amazing and has most of the bases covered, but it makes use of a parent/child device setup on the smartthings side.  Basically, the esp8266 is the parent device, and the parent device handler takes care of network communications.  Child device handlers are then "automagically" added under the parent device depending on which devices you have configured on the arduino side. This was pretty confusing at first, and also cluttered up the smartthings interface with a parent device I had no reason to ever interact with.

So, the answer to that was to hack the parent device handler and the child device handler into a single device handler, removing all of the automagic setup stuff and customized for my single use case: a smartthings switch.

The result is `ST_Anything_ButtonPusher.groovy`.  The device shows up in smartthings as a switch, tapping it sends the on command to the esp8266, the servo swings, presses the button, swings back to the home position, then notifies smartthings that it is now off.

## Final Thoughts

With all of that set up, the Alexa connection is trivial, and now I can say "Alexa, press the button"!  There's still a lot of code I never dug into in the ST_Anything arduino libraries, but it "just worked" so I didn't need to press it.  The example code is all you need to get started.  

I dedicated a lot more time to understanding how the parent device handler code worked so I could customize it.  

If you want to make a smartthings button pusher and stumble across this repo, I hope you find this helpful.
