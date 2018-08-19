/*************************************************************
  Download latest Blynk library here:
    https://github.com/blynkkk/blynk-library/releases/latest

  Blynk is a platform with iOS and Android apps to control
  Arduino, Raspberry Pi and the likes over the Internet.
  You can easily build graphic interfaces for all your
  projects by simply dragging and dropping widgets.

    Downloads, docs, tutorials: http://www.blynk.cc
    Sketch generator:           http://examples.blynk.cc
    Blynk community:            http://community.blynk.cc
    Follow us:                  http://www.fb.com/blynkapp
                                http://twitter.com/blynk_app

  Blynk library is licensed under MIT license
  This example code is in public domain.

 *************************************************************

  This example shows how to synchronize Button widget
  and physical button state.

  App project setup:
    Button widget attached to V2 (Switch mode)
 *************************************************************/

/* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

#ifndef UNIT_TEST
#include <Arduino.h>
#endif
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "************************************";
int pwm[]={1023,1021,1019,1015,1007,991,959,895,767,639,511,383,255,127,0};

// Set your LED and physical button pins here
const int ledWarm = 13;
const int ledCool = 14;

// An IR detector/demodulator is connected to GPIO pin 14(D5 on a NodeMCU
// board).
uint16_t RECV_PIN = 5;
IRrecv irrecv(RECV_PIN);
decode_results results;

int data;

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "*******";
char pass[] = "*******";

void IR_button();


int warm = 0;
int cool = 0;

// Every time we connect to the cloud...
BLYNK_CONNECTED() {
  // Request the latest state from the server
  Blynk.syncVirtual(V1);
  Blynk.syncVirtual(V2);
}

BLYNK_WRITE(V1) {//variable for warm brighness
  warm = param.asInt();
  analogWrite(ledWarm, pwm[warm]);
}

BLYNK_WRITE(V2) {//variable for cool brighness
  cool = param.asInt();
  analogWrite(ledCool, pwm[cool]);
}

void IR_button()
{
  if (irrecv.decode(&results)) {
    data = results.value;
    Serial.println(data,HEX);
    irrecv.resume();  // Receive the next value
    if (data==0xFD12ED)//UP
    {
      // Increase bright
      if (warm >= 14)
      {
        warm=14;
      }
      else
      {
        warm++;
      }
      analogWrite(ledWarm, pwm[warm]);

      // Update variable
      Blynk.virtualWrite(V1, warm);
    }
    
    if (data==0xFD926D)//DOWN
    {
      // Decrease bright
      if (warm <= 0)
      {
        warm=0;
      }
      else
      {
        warm--;
      }
      analogWrite(ledWarm, pwm[warm]);

      // Update variable
      Blynk.virtualWrite(V1, warm);
    }

    if (data==0xFD52AD)//V+
    {
      // Increase bright
      if (cool >= 14)
      {
        cool=14;
      }
      else
      {
        cool++;
      }
      analogWrite(ledCool, pwm[cool]);

      // Update variable
      Blynk.virtualWrite(V2, cool);
    }
    
    if (data==0xFDD22D)//V-
    {
      // Decrease bright
      if (cool <= 0)
      {
        cool=0;
      }
      else
      {
        cool--;
      }
      analogWrite(ledCool, pwm[cool]);

      // Update variable
      Blynk.virtualWrite(V2, cool);
    }
  }
}

void setup()
{
  // Debug console
  Serial.begin(115200);
  irrecv.enableIRIn();  // Start the receiver

  Blynk.begin(auth, ssid, pass);
  // You can also specify server:
  //Blynk.begin(auth, ssid, pass, "blynk-cloud.com", 80);
  //Blynk.begin(auth, ssid, pass, IPAddress(192,168,1,100), 8080);

  pinMode(ledWarm, OUTPUT);
  pinMode(ledCool, OUTPUT);
  analogWrite(ledWarm, pwm[warm]);
  analogWrite(ledCool, pwm[cool]);
}

void loop()
{
  Blynk.run();
  IR_button();
}

