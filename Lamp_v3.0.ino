
/* ESP8266 Lam Controller v2.1 */

#include <ESP8266WiFi.h>        //Library to control ESP8266 WiFi function
#include <BlynkSimpleEsp8266.h> //Blink library for ESP8266

//libraries needed for IR reciever
#ifndef UNIT_TEST
#include <Arduino.h>
#endif
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "YourAuthToken";

/*
  Inversed 15 10-bit predefined brightness level first 8 = 2^n, the rest = 128*n
  Inveresing is required to compesate inversing done by BJT driver
  not inverted value 0,2,4,8,16,32,64, 128, 256,384,512,640,768,896, 1023
*/

//Setting for PIR
//const int PIR = 12; //PIR sensor attached to GPIO 12
//int move1; //tell the controller to ignore/accept PIR data

// GPIO pins used to control main LED panel
const int ledWarm = 13;
const int ledCool = 14;

//Green LED pins for debugging
const int ledGreen = 4;

//Onboard LED pins
const int ledOn1 = 2;

// An IR detector/demodulator is connected to GPIO pin 14(D5 on a NodeMCU
// board).
uint16_t RECV_PIN = 5;
IRrecv irrecv(RECV_PIN);
decode_results results;

int data; //HEX value from IR reciever

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "YourNetworkName";
char pass[] = "YourPassword";

//function for the IR reciever
void IR_button();

//function for the PIR sensor
//void PIR_sense();

//Initial brightness level
int warm = 0;
int cool = 0;
int warm_temp = 0;
int cool_temp = 0;
float warm_cal;
float cool_cal;
int warm_pwm;
int cool_pwm;

int slow;
int interval = 100;
int timer0 = 0;
int timer1 = 0;

// Every time we connect to the cloud...
BLYNK_CONNECTED() {
  // Sync the current virtual pins values to the one in the server
  Blynk.syncVirtual(V1);
  Blynk.syncVirtual(V2);
  Blynk.syncVirtual(V3);
  Blynk.syncVirtual(V4);
}

BLYNK_WRITE(V1) 
{//assign local variable for warm brighness = virtual pins value
  warm = param.asInt();
}

BLYNK_WRITE(V2) 
{//assign local variable for cool brighness = virtual pins value
  cool = param.asInt();
  
}

/*
BLYNK_WRITE(V3) {//assign local variable PIR control
  move1 = param.asInt();
}*/

BLYNK_WRITE(V4) 
{//change brightness respond
  slow = param.asInt();
  
}

BLYNK_WRITE(V5) 
{//change brightness respond
  interval = param.asInt();
  
}

BLYNK_READ(V6)
{
  Blynk.virtualWrite(V6,warm_pwm);
}

BLYNK_READ(V7)
{
  Blynk.virtualWrite(V7,cool_pwm);
}

void IR_button()
{
  if (irrecv.decode(&results)) {//if IR signal is recieved
    
    data = results.value; //store and change HEX data from IR reciever (uint_64 ---> int)
    
    Serial.println(data,HEX);
    irrecv.resume();  // Receive the next value
    
    if (data==0xFD12ED || data == 0xFDD02F || data == 0xFD02FD || data == 0xFD30CF)//Warm+
    {
      // Increase warm brightness
      if (warm >= 4200 || data == 0xFDD02F || data == 0xFD30CF)
      {
        warm=4200;
      }
      else
      {
        warm=warm+200;
      }

      // Update variable
      Blynk.virtualWrite(V1, warm);
    }
    
    if (data == 0xFD926D || data == 0xFD22DD || data == 0xFDF00F || data == 0xFDC837)//Warm-
    {
      // Decrease warm brightness
      if (warm <= 0 || data == 0xFD22DD || data == 0xFDC837)
      {
        warm=0;
      }
      else
      {
        warm=warm-200;
      }

      // Update variable
      Blynk.virtualWrite(V1, warm);
    }

    if (data == 0xFD52AD || data == 0xFDB04F || data == 0xFD02FD || data == 0xFD30CF)//Cool+
    {
      // Increase cool bright
      if (cool >= 4200 || data == 0xFDB04F || data == 0xFD30CF)
      {
        cool=4200;
      }
      else
      {
        cool=cool+200;
      }

      // Update variable
      Blynk.virtualWrite(V2, cool);
    }
    
    if (data == 0xFDD22D || data == 0xFDC23D || data == 0xFDF00F || data == 0xFDC837)//Cool-
    {
      // Decrease cool bright
      if (cool <= 0 || data == 0xFDC23D || data == 0xFDC837)
      {
        cool=0;
      }
      else
      {
        cool=cool-200;
      }
      
      // Update variable
      Blynk.virtualWrite(V2, cool);
    }
  }
}

/*
void PIR_sense()
{
  //change PWM value
  analogWrite(ledWarm, 2);
  
  // Update variable
  Blynk.virtualWrite(V1, pwm[2]);
}
*/

void setup()
{
  // Debug console
  //Serial.begin(115200);

  analogWriteFreq(2000);
  // Start the receiver
  irrecv.enableIRIn();  

  //Start BLynk service
  Blynk.begin(auth, ssid, pass);

  //set GPIO as output
  pinMode(ledWarm, OUTPUT);
  pinMode(ledCool, OUTPUT);
 // pinMode(PIR, INPUT);

  //Innitial LED value (off by default)
  analogWrite(ledWarm, warm_pwm);
  analogWrite(ledCool, cool_pwm);


}

void loop()
{
  Blynk.run();
  IR_button();
  int timer = micros();


  if (slow == 1)
  {
    if ((timer - timer0) >= interval)
    {
      if (warm_temp > warm)
      {warm_temp--;}
      else if (warm_temp < warm)
      {warm_temp++;}
      
      timer0 = timer;
    }
    else
      {warm_temp = warm_temp;}

    if ((timer - timer1) >= interval)
    {
      if (cool_temp > cool)
      {cool_temp--;}
      else if (cool_temp < cool)
      {cool_temp++;}

      timer1 = timer;
    }
    else
      {cool_temp = cool_temp;}
  }
  else if (slow == 0)
  {
    warm_temp = warm;
    cool_temp = cool;
  }

  warm_cal = warm_temp/300;
  cool_cal = cool_temp/300;

  warm_cal = pow(warm_cal,2.626125127);
  warm_pwm = 1023-warm_cal;
  analogWrite(ledWarm, warm_pwm);

  cool_cal = pow(cool_cal,2.626125127);
  cool_pwm = 1023-cool_cal;
  analogWrite(ledCool, cool_pwm);
  
}
