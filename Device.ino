#include <stdio.h>
#include <string.h>
#include <DS3231.h>
#include <Temp.h>
#include <RFade.h>
#include <arduino.h>

Fade fade;
Temp temp;

DS3231 rtc(SDA, SCL);



float Te1;
float Te2;

int airQual;
float airSens = A1;
int ESPPower = 2;

int stateC = 0;
int stateP = -1;
unsigned long uTime;

String mySSID = "SSID";       // WiFi SSID
String myPWD = "123456789"; // WiFi Password
String myAPI = "XXXXXXXXXXXXXXXX";   // API Key
String myHOST = "api.thingspeak.com";
String myPORT = "80";
String myFIELD = "field1";
int sendVal;

String key;

void setup() {

  pinMode(ESPPower, OUTPUT);
  pinMode(airSens, INPUT);
  Serial.begin(115200);
  Serial3.begin(115200);
  rtc.begin();
  // The following lines can be uncommented to set the date and time
  //rtc.setDOW(WEDNESDAY);     // Set Day-of-Week to SUNDAY
  //rtc.setTime(12, 0, 0);     // Set the time to 12:00:00 (24hr format)
  //rtc.setDate(1, 1, 2014);   // Set the date to January 1st, 2014
  Serial.println("\n\n\n\n\n\n\n\n\n\n                                Arduino is Ready...\n\n\n\n");
  digitalWrite(ESPPower, HIGH);
}










char rec() {  // need to make this return a string type and then use it to test the resutl of the serial port.
  String result;
  String rc = "";
  if (Serial3.available() >= 0) {
    while (Serial3.available() >= 0) {
      rc = Serial3.readStringUntil("");
    }
  }
}

void SM() {
  stateP = stateC;
  switch (stateC) {

    case 0://reset
      Serial.println("ESP8266 State Machine Reset.....");
      digitalWrite(ESPPower, LOW);
      stateC = 1;
      break;

    case 1://reset
      delay(50);
      digitalWrite(ESPPower, HIGH);
      Serial.find("ready");
      Serial3.print("ATE0\r\n");

      stateC = 2;
      break;

    case 2://res
    et
      stateC = 3;
      break;

    case 3://reset
      stateC = 4;
      break;

    case 4://reset
      stateC = 5;
      break;

    case 5://reset
      stateC = 6;
      break;

    case 6://reset
      stateC = 7;
      break;

    case 7://reset
      stateC = 8;
      break;

    case 8://reset
      stateC = 9;
      break;

    case 9://reset
      stateC = 10;
      break;

    case 10://reset
      stateC = 0;
      break;
  }
}



void Sensors() {
  //temp:
  Te1 = rtc.getTemp();
  Te2 = temp.tempRead();
  Serial.println(Te1);
  Serial.println(Te2);

  //air:
  airQual = analogRead(airSens);
  Serial.println(airQual);
}


String Cat() {
  //("AT+CWJAP=\""+ mySSID +"\",\""+ myPWD +"\"", 1000,")
  //AT+CWMODE=1

  //espData("AT+CIPMUX=1", 1000, DEBUG);       //Allow multiple connections
  //    espData("AT+CIPSTART=0,\"TCP\",\""+ myHOST +"\","+ myPORT, 1000, DEBUG);
  //    espData("AT+CIPSEND=0," +String(sendData.length()+4),1000,DEBUG);
  //    espSerial.find(">");
  //    espSerial.println(sendData);
  //    Serial.print("Value to be sent: ");
  //    Serial.println(sendVal);
  //
  //espData("AT+CIPCLOSE=0",1000,DEBUG);
  //String payload;
  //payload = Serial.print("AT+CIPSTART=”TCP”,”184.106.153.149”,80"); // the thingspeak address and keys
  //payload += "GET /update?api_key="+ myAPI +"&"+ myFIELD +"="+String(sendVal); // next thing to cat on to it
  //payload += ; // next thing to cat on to it
  //payload += ; // next thing to cat on to it


  //  return payload;
}

void loop() {

  fade.breathe(); // Fade led with a sine wave value.
  //rec(); // collect serial data

  Sensors(); // read sensor data
  if (stateP != stateC) { // check to see if the state has changed from last.
    Serial.print("State of machine is: ");
    Serial.println(stateC);
    Serial.print("The time is now ");
    Serial.print(rtc.getTimeStr());
    Serial.print(" and the current temperature is ");
    Serial.print(rtc.getTemp());
    Serial.println(" Degrees Centergrade");
  }

  SM(); //run the state machine
  //Cat();  // concatenate together all the data needed to send



  delay(500); // slow down the program for building it
}

