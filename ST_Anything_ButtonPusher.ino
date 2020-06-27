// ST_Anything_ButtonPusher.ino - Configures an ESP8266 to connect to SmartThings Hub over wifi as a RelaySwitch
// (switch that turns itself off automatically after bring turned on)
// When turned on, the S_ServoSwing library turns a servo to the specified angle, then back to its original position

// SmartThings Library for ESP8266WiFi
#include <SmartThingsESP8266WiFi.h>

// ST_Anything Libraries
#include <Constants.h>       //Constants.h is designed to be modified by the end user to adjust behavior of the ST_Anything library
#include <Device.h>          //Generic Device Class, inherited by Sensor and Executor classes
#include <Sensor.h>          //Generic Sensor Class, typically provides data to ST Cloud (e.g. Temperature, Motion, etc...)
#include <Executor.h>        //Generic Executor Class, typically receives data from ST Cloud (e.g. Switch)
#include <InterruptSensor.h> //Generic Interrupt "Sensor" Class, waits for change of state on digital input
#include <PollingSensor.h>   //Generic Polling "Sensor" Class, polls Arduino pins periodically
#include <Everything.h>      //Master Brain of ST_Anything library that ties everything together and performs ST Shield communications

#include <S_ServoSwing.h>    //Implements a Sensor to turn a servo to a specified angle, then back again

// servo PWM wire is connected to D1
#define PIN_SERVO          D1

//ESP8266 WiFi Information
String str_ssid     = "";
String str_password = "";
IPAddress ip(192, 168, 1, 128);       //Device IP Address
IPAddress gateway(192, 168, 1, 1);    //Router gateway
IPAddress subnet(255, 255, 255, 0);   //LAN subnet mask
IPAddress dnsserver(192, 168, 1, 1);  //DNS server
const unsigned int serverPort = 8090; // port to run the http server on

// Smarthings Hub Information
IPAddress hubIp(192, 168, 7, 3);  // smartthings hub ip       //  <---You must edit this line!
const unsigned int hubPort = 39500; // smartthings hub port

// this is a sniffer to monitor data being sent to ST
void callback(const String &msg)
{
  Serial.print(F("ST_Anything Callback: Sniffed data = "));
  Serial.println(msg);

  //TODO:  Add local logic here to take action when a device's value/state is changed

  //Masquerade as the ThingShield to send data to the Arduino, as if from the ST Cloud (uncomment and edit following line)
  //st::receiveSmartString("Put your command here!");  //use same strings that the Device Handler would send
}

//Arduino Setup() routine
void setup()
{
  //
  const unsigned int startAngle = 50;
  const unsigned int endAngle = 180;
  const unsigned int theDelay = 10;

  // delcare the servo
  static st::S_ServoSwing          sensor1(F("relaySwitch"), PIN_SERVO, startAngle, endAngle, theDelay);

  //  Configure debug print output from each main class
  st::Everything::debug=true;
  st::Executor::debug=true;
  st::Device::debug=true;
  st::PollingSensor::debug=true;
  st::InterruptSensor::debug=true;

  //Initialize the "Everything" Class

  //Initialize the optional local callback
  st::Everything::callOnMsgSend = callback;

  //Create the SmartThings ESP8266WiFi Communications Object
  st::Everything::SmartThing = new st::SmartThingsESP8266WiFi(str_ssid, str_password, ip, gateway, subnet, dnsserver, serverPort, hubIp, hubPort, st::receiveSmartString);

  //Run the Everything class' init() routine which establishes WiFi communications with SmartThings Hub
  st::Everything::init();

  //Add each sensor to the "Everything" Class
  st::Everything::addSensor(&sensor1);

  //Initialize each of the devices which were added to the Everything Class
  st::Everything::initDevices();

}

void loop()
{
  st::Everything::run();
}
