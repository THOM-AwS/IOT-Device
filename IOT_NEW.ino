int state_SMESP = 0;
int state_prev_SMESP = 0;
int val_SMESP = 0;
//digitalWrite(ESPPower, HIGH);
void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:

SMESP();

}

void SMESP(){
  state_prev_SMESP = state_SMESP;
  switch(state_SMESP){
  
  case 0:// Reset point
    state_SMESP = 1;
  break;
  
  case 1: // see if responsive
    Serial3.print("AT\r\n");
    state_SMESP = 2;
  break;
  
  case 2:// get what is returned from the ESP8266
    messageHandler();
  break;
  
  case 3:// get and display the IP Address
      Serial3.print("AT+CIFSR\r\n");
      state_SMESP = 2;
  break;
  
  case 4:
  break;
  
  case 5: // Error reset ESP8266
    digitalWrite(ESPPower, LOW);
    delay(50);
    digitalWrite(ESPPower, HIGH);
    state_SMESP = 0;
  break;
}
}

void messageHandler(){ // gets what is sitting on the serial port and parses.
  String message=(""); // initialise a string variable for returning the message

  if (Serial3.available()){
    while (Serial3.available()){
      char c = (Serial3.read());
      message += c;
      Serial.println(c);
      }
    }  
    else if (Serial3.available()==0){
        Serial.println("no Response here...");
      }
  String str(message);
//  Serial.println("________________________________>>>>");
//  Serial.println(str);
//  Serial.println("________________________________<<<<");
  if (str.indexOf(kwBusy)>-1){
      //Serial.print("index of " + kwBusy + " is: ");
      //Serial.println(str.indexOf(kwBusy));
      Serial.println(">>>");
      Serial.println(kwBusy);
      Serial.println("<<<");
    }
  if (str.indexOf(kwOK)>-1){
      //Serial.print("index of " + kwOK + " is: ");
      //Serial.println(str.indexOf(kwOK));
      Serial.println(">>>");
      Serial.println(kwOK);
      Serial.println("<<<");
      if ((state_prev_SMESP) == 1){
        state_SMESP = 3;
      }
    }
  if (str.indexOf(kwDISC)>-1){
      //Serial.print("index of " + kwDISC + " is: ");
      //Serial.println(str.indexOf(kwDISC));
      Serial.println(">>>");
      Serial.println(kwDISC);
      Serial.println("<<<");
      state_SMESP = 5;
    }
  if (str.indexOf(kwConnect)>-1){
      //Serial.print("index of " + kwConnect + " is: ");
      //Serial.println(str.indexOf(kwConnect));
      Serial.println(">>>");
      Serial.println(kwConnect);
      Serial.println("<<<");
    }
  if (str.indexOf(kwError)>-1){
      //Serial.print("index of " + kwError + " is: ");
      //Serial.println(str.indexOf(kwError));
      Serial.println(">>>");
      Serial.println(kwError);
      Serial.println("<<<");
      state_SMESP = 5;
    }
  if (str.indexOf(kwGotIP)>-1){
      //Serial.print("index of " + kwGotIP + " is: ");
      //Serial.println(str.indexOf(kwGotIP));
      Serial.print(Serial3.println("AT+CIFSR\r\n"));
      Serial.println(">>>");
      Serial.println(kwGotIP);
      Serial.println("<<<");
    }
  if (str.indexOf(kwIP)>-1){
      //Serial.print("index of " + kwIP + " is: ");
      //Serial.println(str.indexOf(kwIP));
      Serial.println(">>>");
      Serial.println(kwIP + str.substring(indexOf("\"")),indexOf("\"" + 1));
      Serial.println("<<<");
    }
}
