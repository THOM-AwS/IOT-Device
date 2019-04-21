#include <stdio.h>
#include <string.h>
#include <DS3231.h>
#include <Temp.h>
#include <RFade.h>
#include <arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

//#define DHTPIN 2
//#define DHTTYPE DHT11
#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);


WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
Fade fade;
Temp temp;

DS3231 rtc(SDA, SCL);

//averaging code here:
const int numSoundReadings = 5;
const int numAirReadings = 5;
const int numTempReadings = 5;
 
int soundReadings[numSoundReadings];      // the soundReadings from the analog input
int soundReadIndex = 0;              // the index of the current reading
int soundTotal = 0;                  // the running soundTotal

int airReadings[numAirReadings];      // the soundReadings from the analog input
int airReadIndex = 0;              // the index of the current reading
int airTotal = 0;                  // the running soundTotal

int tempReadings[numTempReadings];      // the soundReadings from the analog input
int tempReadIndex = 0;              // the index of the current reading
int tempTotal = 0;                  // the running soundTotal


int soundAv = 0;                // the average sound
int airAv = 0;                  // the average air
float tempAv = 0;                 // the average temp


//sensor declarations
int Te1;
float Te2;
int airQual;
int airSens = A1;
int soundSens = A2;
int soundVol;
long SR; // Seconds remaining on display
long CDT; //count down timer for display

int stateC = 0;
int stateP = -1;
unsigned long uTime;

bool inverted = false;

String mySSID = "ESP";       // WiFi SSID
String myPWD = "1234567890"; // WiFi Password
String myAPI = "FQDMKIUBU5BEC0JJ";   // API Key
String myHOST = "api.thingspeak.com";
String myPORT = "80";
String myFIELD1 = "field1";
String myFIELD2 = "field2";
String myFIELD3 = "field3";
String myFIELD4 = "field4";
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
  Wire.begin();
  display.begin(SSD1306_SWITCHCAPVCC, 0x3c);// initialize with the I2C addr 0x3C

  //init smoothing array for sound
  for (int thisReading = 0; thisReading < numSoundReadings; thisReading++) {
    soundReadings[thisReading] = 0;
  }
  
  pinMode(soundSens, INPUT);
  pinMode(airSens, INPUT);
  Serial.begin(115200);
  Serial3.begin(115200);
  rtc.begin();
  // The following lines can be uncommented to set the date and time
  //rtc.setDOW(THURSDAY);     // Set Day-of-Week to SUNDAY
  //rtc.setTime(11, 11, 0);     // Set the time to 12:00:00 (24hr format)
  //rtc.setDate(4, 4, 2019);   // Set the date to January 1st, 2014
  Serial.println("\n\n\n\n\n\n\n\n\n\n                                Arduino is Ready...\n\n\n\n");
  while (Serial3.available()) {
    Serial.read();
  }
}
void Display() {

  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(5, 0);
  display.print("Temp: ");
  display.print(tempAv);
  display.print("c");
  //  display.print(" T2: ");
  //  display.print(Te1);
  //  display.print("c");
  display.setCursor(5, 10);
  display.print("Air: ");
  display.print(airAv);
  display.setCursor(5, 20);
  display.print("Time: ");
  display.print(rtc.getTimeStr());
  display.setCursor(5, 30);
  display.print("State: ");
  display.print(stateC);
  display.print("  dB: ");
  display.print(soundAv);
  display.setCursor(5, 40);
  display.print("Remaining: ");
  display.print(SR / 1000);
  display.print(".");
  display.print((SR % 1000) / 100);
  display.setCursor(5, 50);
  display.print("Clock: ");
  display.print(millis());


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
      TOInterv = 2000;
      if (timeout()) {
        Serial.println();
        Serial.println("TIMEOUT");
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
      timeClient.begin();
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
        if (inverted) {
          display.invertDisplay(false);
          inverted = false;
        }
        else {
          display.invertDisplay(true);
          inverted = true;
        }
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
      if (TOInterv != 60000) {
        TOInterv = 60000;
        CDT = millis();
      }
      if (timeout()) {
        Serial.println();
        Serial.println("TIMEOUT");
        stateC = 10;
        SR = 0;
        break;
      }
      else {
        SR = ((CDT + TOInterv) - millis());
        

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

  // subtract the last reading:
  tempTotal = tempTotal - tempReadings[tempReadIndex];
  // read from the sensor:
  tempReadings[tempReadIndex] = Te2;
  // add the reading to the tempTotal:
  tempTotal = tempTotal + tempReadings[tempReadIndex];
  // advance to the next position in the array:
  tempReadIndex = tempReadIndex + 1;
 
  // if we're at the end of the array...
  if (tempReadIndex >= numTempReadings) {
    // ...wrap around to the beginning:
    tempReadIndex = 0;
  }
 
  // calculate the average:
  tempAv = tempTotal / numTempReadings;


  //air:
  airQual = analogRead(airSens);

  // subtract the last reading:
  airTotal = airTotal - airReadings[airReadIndex];
  // read from the sensor:
  airReadings[airReadIndex] = airQual;
  // add the reading to the airTotal:
  airTotal = airTotal + airReadings[airReadIndex];
  // advance to the next position in the array:
  airReadIndex = airReadIndex + 1;
 
  // if we're at the end of the array...
  if (airReadIndex >= numAirReadings) {
    // ...wrap around to the beginning:
    airReadIndex = 0;
  }
 
  // calculate the average:
  airAv = airTotal / numAirReadings;




  //sound:
  soundVol = analogRead(soundSens);
  
  // subtract the last reading:
  soundTotal = soundTotal - soundReadings[soundReadIndex];
  // read from the sensor:
  soundReadings[soundReadIndex] = soundVol;
  // add the reading to the soundTotal:
  soundTotal = soundTotal + soundReadings[soundReadIndex];
  // advance to the next position in the array:
  soundReadIndex = soundReadIndex + 1;
 
  // if we're at the end of the array...
  if (soundReadIndex >= numSoundReadings) {
    // ...wrap around to the beginning:
    soundReadIndex = 0;
  }
 
  // calculate the average:
  soundAv = soundTotal / numSoundReadings;
  
  

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
  sendData += "&";
  sendData += myFIELD4;
  sendData += "=";
  sendData += String(soundVol);
  sendData += "\r\n\r\n";
}
void loop() {
  SM(); //run the state machine

  Display();

  SM(); //run the state machine

  display.display();

  SM(); //run the state machine

  Sensors();

  SM(); //run the state machine

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


