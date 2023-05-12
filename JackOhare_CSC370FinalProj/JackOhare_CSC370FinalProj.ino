#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "WiFiEsp.h"
//#ifndef HAVE_HWSERIAL1
#include "SoftwareSerial.h"
SoftwareSerial softserial(A9, A8);  // A9 to ESP_TX, A8 to ESP_RX by default
//#endif
LiquidCrystal_I2C lcd(0x3f, 16, 2); 
//ESP Connection
int    HTTP_PORT   = 80;
String HTTP_METHOD = "GET";
char   HOST_NAME[] = "maker.ifttt.com";
String PATH_NAME   = "/trigger/send-email/with/key/INPUTKEYHERE"; 
String queryString = "?value1=27&value2=69&value3=21"; 

WiFiEspClient client;
char ssid[] = "Joe";     
char pass[] = "bruh1234";
int status = WL_IDLE_STATUS;
unsigned long timeOn = 0, timeOff = 0, timeOffStart = 0,timeOnStart=0; 
//Temp Buttons
const int incTempPin = 13;
const int DecTempPin = 12;
const int pausePin = 9;
const int submitPin = 10;
int buttonStateInc = 0;
int buttonStateDec = 0;
int pauseState = 0;
int submitState = 0;
bool isOn = false;
//Tempo Buzzer
int pinBuzzer = 22;
//Tempo Result
float tempo = 80;
float interval = 500;

//LCD
byte musicWhole[] = {
  B00000,
  B00000,
  B00011,
  B00101,
  B00100,
  B00100,
  B11100,
  B11100
};
/* Changes tempo based on if button is clicked;
   Works in both positive and negative tempo changes*/
void changeTmp(int buttonSts, float tmpChange)
{
    if (buttonSts == HIGH) {

  } else {

    if(tempo<=200 && tempo>=60)
    {
    tempo +=tmpChange;
    }
  }
}


void setup() {
  //LCD Display 
  lcd.backlight();
  lcd.init();
  lcd.createChar(0,musicWhole);
  //Serial control
  Serial.begin(9600);
  
  //esp wifi control
  softserial.begin(115200);
  softserial.write("AT+CIOBAUD=9600\r\n");
  softserial.write("AT+RST\r\n");
  softserial.begin(9600);  // initialize serial for ESP module
  WiFi.init(&softserial);  // initialize ESP module
    // check for the presence of the shield
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue
    while (true)
      ;
  }


    // attempt to connect to WiFi network
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network
    status = WiFi.begin(ssid, pass);
  }


   Serial.println("You're connected to the network");
  //intialize buttons
  pinMode(incTempPin, INPUT);
  pinMode(DecTempPin, INPUT);
  pinMode(pausePin, INPUT);
  pinMode(submitPin, INPUT);
  //for speaker initialization
  pinMode(pinBuzzer,OUTPUT);
  
}

void loop() {  
  //check states of button
  buttonStateInc = digitalRead(incTempPin);
  buttonStateDec = digitalRead(DecTempPin);
  pauseState = digitalRead(pausePin);
  submitState = digitalRead(submitPin);
  //Change tempos
  changeTmp(buttonStateInc, 4);
  changeTmp(buttonStateDec,-4);
  //pause
  if (pauseState != HIGH) 
  {    
    
    //If on record how long its been playing and then turn off the speaker 
    if(isOn)
    {
      timeOnStart = millis();
      isOn = false;
      timeOff = timeOff + (timeOnStart-timeOffStart);
      Serial.println(timeOff);
      delay(1000);
    }
    //else If off record how long its not been playing and then turn on the speaker 
    else if(!isOn)
    {
      timeOffStart = millis();
      isOn = true;
      timeOn = timeOn + (timeOffStart-timeOnStart);
      Serial.println(timeOn);
       delay(1000);
    }
    Serial.println(isOn); //to test whether or not it turns on
  }
    //Submission process
    if(submitState != HIGH)
   {
    if(!isOn)
    {timeOffStart = millis();
    timeOn = timeOn + (timeOffStart-timeOnStart);}
    else
    { timeOnStart = millis();
    timeOff = timeOff + (timeOnStart-timeOffStart);
    }
    delay(1000);       
    queryString = "?value1="+String((timeOn+timeOff)/60000) +"&value2="+String(timeOn/60000) +"&value3="+String(timeOff/60000);
    lcd.clear();
    
    lcd.print("Attempting...");       // connect to web server on port 80:
    if (client.connect(HOST_NAME, HTTP_PORT)) {
    // if connected:
    Serial.println("Connected to server");
    // make a HTTP request:
    // send HTTP header
    client.println("GET " + PATH_NAME + queryString + " HTTP/1.1");
    client.println("Host: " + String(HOST_NAME));
    client.println("Connection: close");
    client.println(); // end HTTP header

    while (client.connected()) {
      if (client.available()) {
        // read an incoming byte from the server and print it to serial monitor:
        char c = client.read();
        Serial.print(c);
      }
    }

    // the server's disconnected, stop the client:
    client.stop();
    Serial.println();
    Serial.println("disconnected");
    lcd.clear();
    lcd.print("Success!");
    delay(5000);
    lcd.clear();
  } else {// if not connected:
    Serial.println("connection failed");
    lcd.clear();
    lcd.print("Connection Error!");
    delay(5000);
    lcd.clear();
  }
  
  }
  
  //sets tempo to correct BPM 
  interval = (60/tempo)*1000;
  //Visually inputs to LCD screen 
  lcd.setCursor(0, 0);
  lcd.write(0);
  lcd.print(":");
  lcd.print(tempo);
  //if is on, play metronome
  if(isOn)
  {
    tone(pinBuzzer,500);
    delay(interval);
    noTone(pinBuzzer);
    delay(1);  
  }
 
}
