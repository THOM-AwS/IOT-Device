#include <stdio.h>
#include <string.h>
#include <DS3231.h>
#include <Temp.h>
#include <RFade.h>
#include <arduino.h>

Fade fade;
Temp temp;

DS3231 rtc(SDA, SCL);



int Te1;
int Te2;
int airQual;
float airSens = A1;

int stateC = 0;
int stateP = -1;
unsigned long uTime;

String mySSID = "Esp";       // WiFi SSID
String myPWD = "1234567890"; // WiFi Password
String myAPI = "FQDMKIUBU5BEC0JJ";   // API Key
String myHOST = "api.thingspeak.com";
String myPORT = "80";
String myFIELD1 = "field1";
String myFIELD2 = "field2";
String myFIELD3 = "field3";
String sendData;

char kwBusy[] = "busy p...";
char kwOK[] = "OK";
char kwSendOK[] = "SEND OK";
char kwClosed[] = "CLOSED";
char kwWIFICONNECTED[] = "WIFI CONNECTED";
char kwGotIP[] = "WIFI GOT IP";
char kwError[] = "ERROR";
char kwConnect[] = "CONNECT";
char kwIP[] = "+CIPSTA:ip:";
char kwGW[] = "+CIPSTA:gateway:";
char kwReady[] = "ready";
char LocalIP[] = "";
char Gateway[] = "";

unsigned long TOpms;
unsigned long TOcms;
unsigned long TOInterv;

const byte numChars = 3000; // so the complier knows how many chars the array size needs to be.
char receivedChars[numChars]; // an array to store the received data.
boolean newData = false;

void setup() {

  pinMode(airSens, INPUT);
  Serial.begin(115200);
  Serial3.begin(115200);
  rtc.begin();
  // The following lines can be uncommented to set the date and time
  //rtc.setDOW(WEDNESDAY);     // Set Day-of-Week to SUNDAY
  //rtc.setTime(12, 0, 0);     // Set the time to 12:00:00 (24hr format)
  //rtc.setDate(1, 1, 2014);   // Set the date to January 1st, 2014
  Serial.println("\n\n\n\n\n\n\n\n\n\n                                Arduino is Ready...\n\n\n\n");
  while (Serial3.available()) {
    Serial.read();
  }

}

bool timeout() {
  TOcms = millis();
  if ((TOcms - TOpms) >= TOInterv) { //  watchdog
    TOpms = millis();
    return true;
  }
  else {
    return false;
  }
}
bool rec(char * TargetStr) {  // need to make this return a string type and then use it to test the resutl of the serial port.
  int TargetLen = strlen(TargetStr);
  static byte ndx = 0;  // this is a byte that saves the index of the array where the bytes will be stored.
  static byte TargetRecNDX = 0;
  char rc; //this is the char variable that is allocated the actual char that is read before its passed to the array of 'recievedChars.
  if (Serial3.available() > 0) { //If there is something on the serial port,
    while (Serial3.available() > 0) { // do this while there is,
      rc = Serial3.read(); // the recieved char is allocaed what is read off the serial port.
      Serial.print(rc);
      if (rc == TargetStr[TargetRecNDX]) {
        TargetRecNDX++;
      }
      else {
        TargetRecNDX = 0;
      }
      if (TargetRecNDX >= TargetLen) {
        //Serial.println();
        TOcms = millis();
        Serial.println();
        Serial.print("Found: ");
        Serial.println(TargetStr);
        Serial.print("Duration: ");
        Serial.println(TOcms - TOpms);
        return true;
      }
    }
  }
  return false;
}
void SM() {

  stateP = stateC;
  switch (stateC) {

    case 0://reset
      Serial.println("ESP8266 State Machine Reset.....");
      Serial3.print("AT+RST\r\n");
      stateC = 1;
      TOpms = millis();
      break;

    case 1:
      if (rec(kwReady)) {
        stateC = 2;
        TOpms = millis();
        break;
      }
      // wait for the kw timer
      TOInterv = 400;
      if (timeout()) {
        stateC = 0;
        break;
      }
      else {
        break;
      }

    case 2:
      if (rec(kwWIFICONNECTED)) {
        stateC = 3;
        TOpms = millis();
        break;
      }
      // wait for the kw timer
      TOInterv = 5000;
      if (timeout()) {
        Serial.println();
        Serial.println("TIMEOUT");
        stateC = 20;
        break;
      }
      else {
        
        break;
      }

    case 3:
      if (rec(kwGotIP)) {
        stateC = 4;
        TOpms = millis();
        break;
      }
      // wait for the kw timer
      TOInterv = 1000;
      if (timeout()) {
        Serial.println();
        Serial.println("TIMEOUT");
        stateC = 0;
        break;
      }
      else {
        
        break;
      }

    case 4:
      Serial3.print("ATE0\r\n");
      stateC = 5;
      TOpms = millis();
      break;

    case 5:
    delay(300);
      if (rec(kwOK)) {
        stateC = 6;
        TOpms = millis();
        break;
      }
      // wait for the kw timer
      TOInterv = 100;
      if (timeout()) {
        Serial.println();
        Serial.println("TIMEOUT");
        stateC = 0;
        break;
      }
      else {
        
        break;
      }

    case 6:
      Serial3.print("AT+CIPMODE=0\r\n");
      Serial.print("AT+CIPMODE=0\r\n");
      stateC = 7;
      
      break;

    case 7:
      delay(10);
      if (rec(kwOK)) {
        stateC = 8;
        TOpms = millis();
        break;
      }
      // wait for the kw timer
      TOInterv = 100;
      if (timeout()) {
        Serial.println();
        Serial.println("TIMEOUT");
        stateC = 0;
        break;
      }
      else {
        
        break;
      }

    case 8:
      Serial3.print("AT+CIPMUX=0\r\n");
      Serial.print("AT+CIPMUX=0\r\n");
      stateC = 9;
      TOpms = millis();
      break;

    case 9:
      delay(10);
      if (rec(kwOK)) {
        stateC = 10;
        TOpms = millis();
        break;
      }
      // wait for the kw timer
      TOInterv = 100;
      if (timeout()) {
        Serial.println();
        Serial.println("TIMEOUT");
        stateC = 0;
        break;
      }
      else {
        
        break;
      }

    case 10:
      Serial.println("Connecting to " + myHOST + " now...");
      Serial3.print("AT+CIPSTART=\"TCP\",\"" + myHOST + "\"," + myPORT + "\r\n");
      Serial.print("AT+CIPSTART=\"TCP\",\"" + myHOST + "\"," + myPORT + "\r\n");
      stateC = 11;
      TOpms = millis();
      break;

    case 11:
      if (rec(kwConnect)) {
        stateC = 12;
        TOpms = millis();
        break;
      }
      // wait for the kw timer
      TOInterv = 1000;
      if (timeout()) {
        Serial.println();
        Serial.println("TIMEOUT");
        stateC = 16;
        break;
      }
      else {
        
        break;
      }

    case 12:
      Serial.println("Sending Length...");
      Cat();  // concatenate together all the data needed to send
      Serial3.println("AT+CIPSEND=" + String(sendData.length()));
      Serial.print("AT+CIPSEND=" + String(sendData.length()));
      stateC = 13;
      break;

    case 13:
      if (rec(">")) {
        stateC = 14;
        TOpms = millis();
        break;
      }
      // wait for the kw timer
      TOInterv = 100;
      if (timeout()) {
        Serial.println();
        Serial.println("TIMEOUT");
        stateC = 16;
        break;
      }
      else {
        
        break;
      }

    case 14:
      delay(1000);
      Serial.println("Sending payload...");
      Serial3.print(sendData);
      Serial.print(sendData);
      stateC = 15;
      break;

    case 15:
      if (rec(kwSendOK)) {
        stateC = 16;
        TOpms = millis();
        break;
      }
      // wait for the kw timer
      TOInterv = 4000;
      if (timeout()) {
        Serial.println();
        Serial.println("TIMEOUT");
        stateC = 16;
        break;
      }
      else {
        
        break;
      }

    case 16:
      Serial.println("Closing Connection and restarting...");
      Serial.print("AT+CIPCLOSE\r\n");
      Serial3.print("AT+CIPCLOSE\r\n");
      stateC = 17;
      break;

    case 17:
      if (rec(kwClosed)) {
        stateC = 30;
        TOpms = millis();
        Serial.println("");
        break;
      }
      // wait for the kw timer
      TOInterv = 3000;
      if (timeout()) {
        Serial.println("TIMEOUT");
        stateC = 30;
        break;
      }
        break;

case 30:
  TOInterv = 60000;
      if (timeout()) {
        Serial.println();
        Serial.println("TIMEOUT");
        stateC = 10;
        break;
      }
      else {
        
        break;
      }


    case 20:
      Serial.println("Not Connected, Trying to connect now.....");
      Serial3.print("AT+CWJAP=\"" + mySSID + "\",\"" + myPWD + "\"");
      stateC = 2;
      break;
  }
}
void Sensors() {

  //temp:
  Te1 = rtc.getTemp();
  Te2 = temp.tempRead();

  //air:
  airQual = analogRead(airSens);

}


void Cat() {

  sendData = "GET /update?api_key=";
  sendData += myAPI;
  sendData += "&";
  sendData += myFIELD1;
  sendData += "=";
  sendData += String(Te2);
  sendData += "&";
  sendData += myFIELD2;
  sendData += "=";
  sendData += String(Te1);
  sendData += "&";
  sendData += myFIELD3;
  sendData += "=";
  sendData += String(airQual);
  sendData +="\r\n\r\n";
}

void loop() {
  Sensors();
  fade.breathe(); // Fade led with a sine wave value.
  
  SM(); //run the state machine
  
  if (stateP != stateC) { // check to see if the state has changed from last.
    Serial.println();
    Serial.println();
    Serial.print("State of machine is: ");
    Serial.print(stateC);
    Serial.print(" The time is now ");
    Serial.print(rtc.getTimeStr());
    Serial.print(" and the current temperature is ");
    Serial.print(rtc.getTemp());
    Serial.println(" Degrees Centergrade");
    Serial.println();
  }
}


/*
  AT+CIPSTART="TCP","api.thingspeak.com",80
  AT+CIPSEND=46
  GET /update?api_key=FQDMKIUBU5BEC0JJ&field1=21
  AT+CIPCLOSE
  GET https://api.thingspeak.com/update?api_key=FQDMKIUBU5BEC0JJ&field1=00
*/
