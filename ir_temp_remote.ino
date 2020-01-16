//DHT LIB
#include "DHT.h"
#define DHTPIN 10
#define DHTTYPE DHT22   // DHT 11
DHT dht(DHTPIN, DHTTYPE);

//IR Commands
#include <IRremote.h>
IRsend irsend;

//AVR Watchdog sleeper
#include <Adafruit_SleepyDog.h>

//Digital pins as serial pins
#include <SoftwareSerial.h>
int RXPIN = 16;
int TXPIN = 15;
SoftwareSerial mySerial(RXPIN, TXPIN); //RX, TX

//Adjustment Variables
float daytemp = 66.00;
float nighttemp = 62.00;
int sleepmins = 4;

//Global Variable Initilization
float controltemp = daytemp;
boolean heaterstatus = false;
boolean btsignal = false;
String input = "false";
float heaterset = controltemp;

void setup() {
  Serial.begin(9600);
  Serial.setTimeout(10);
  mySerial.begin(9600);
  dht.begin();
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  //Need to calibrate heaterset intially
  heaterset = 80;
  IRCommands("power");
  heaterstatus = true;
  while (heaterset > 59){
    IRCommands("tempdown");
  }
  changetemp();
  digitalWrite(LED_BUILTIN, HIGH);
}

void loop() {
  Serial.println("beginning");
  delay(1000);
//Take temp measurements
  float h = dht.readHumidity();
  float f = dht.readTemperature(true);
  float hif = dht.computeHeatIndex(f, h);
  Serial.println(h);
  Serial.println(f);
  Serial.print(hif);
  delay(500);
//Check for commands over serial port from bluetooth comm.
 char header;
 while(mySerial.available()) {
   header = mySerial.read();// read the incoming data as string 
   if (header == 'x'){
     btexecute();
   }
 }

//Decide if heater needs to be switched off if it is too hot
 if (hif > controltemp) {
   if (heaterstatus == true){
     IRCommands("power");
     heaterstatus = false;
    }
  }

//Decide if heater needs to be switched on if it is too cold
  if (hif < controltemp) {
    if (heaterstatus == false){
      IRCommands("power");
      heaterstatus = true;
    }
  }

  delay(100);
  //sleepytime();
}

void sleepytime(){
  int loops = (sleepmins*60)/9;
  loops = 2;
  digitalWrite(LED_BUILTIN, LOW); // Show we're asleep
  for (int s = 0; s < loops; s++){
    int sleepMS = Watchdog.sleep();
    #if defined(USBCON) && !defined(USE_TINYUSB)
    USBDevice.attach();
    #endif
  }
  digitalWrite(LED_BUILTIN, HIGH); // Show we're awake again
}

void btexecute(){
  //Time to read the command string!
  int btnum;
  char commandchar;
  String btnumstr = "";
  while(mySerial.available()) {
    char commandchar = mySerial.read();
    delay(50);
    //btnum value assign
      char num1 = mySerial.read();
      char num2 = mySerial.read();
      btnumstr.concat(num1);
      btnumstr.concat(num2);
      btnum = btnumstr.toInt();
    
    //Now executes received command
    if (commandchar == 't'){
      if (btnum > 12){
        mySerial.println("invalid");
      }
      mySerial.println(btnum);
      for (btnum; btnum > 0; btnum--){
        IRCommands("timer");
      }
    }
    if (commandchar == 'd'){
      if (btnum < 59 || btnum > 80){
        mySerial.println("invalid");
      }
      mySerial.println(btnum);
      controltemp = btnum;
      changetemp();
    }
    if (commandchar == 'n'){
      mySerial.println("pm temp");
      controltemp = nighttemp;
      changetemp();
    }
    if (commandchar == 'm'){
      mySerial.println("am temp");
      controltemp = daytemp;
      changetemp();
    }
    if (commandchar == 'p'){
      IRCommands("power");
      IRCommands("high");
      int highmins = 10;
      mySerial.println("preheating");
      delay(60000*highmins);
      IRCommands("low");
    }
    if (commandchar == 'o'){
      IRCommands("power");
      heaterstatus = true;
      mySerial.println("on");
    }
    if (commandchar == 'f'){
      IRCommands("power");
      heaterstatus = false;
      mySerial.println("off");
    }
  }
}

void changetemp(){
  if(heaterstatus == false){
    IRCommands("power");
    heaterstatus = true;
  }
  while(heaterset != controltemp){
    if (heaterset < controltemp){
      IRCommands("tempup");
    }
    if (heaterset > controltemp){
      IRCommands("tempdown");
    }
  }
}

void IRCommands(String input){
  if (input == "power"){
      for (int i = 0; i < 3; i++) {
        irsend.sendNEC(0x806F9867,32);
        delay(40);
      }
  }
    if (input == "tempup"){
      for (int i = 0; i < 3; i++) {
        irsend.sendNEC(0x806F48B7,32);
        delay(40);
      }
      heaterset++;
  }
    if (input == "tempdown"){
      for (int i = 0; i < 3; i++) {
        irsend.sendNEC(0x806F6897,32);
        delay(40);
      }
      heaterset--;
  }
    if (input == "low"){
      for (int i = 0; i < 3; i++) {
        irsend.sendNEC(0x806F28D7,32);
        delay(40);
      }
  }
    if (input == "high"){
      for (int i = 0; i < 3; i++) {
        irsend.sendNEC(0x806F08F7,32);
        delay(40);
      }
  }
    if (input == "timer"){
      for (int i = 0; i < 3; i++) {
        irsend.sendNEC(0x806F10EF,32);
        delay(40);
      }
  }
}

