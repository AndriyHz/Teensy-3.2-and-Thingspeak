
/* Arduino+esp8266 thingSpeak example  
 * Example name = "Write temperature and humidity to Thingspeak channel"
 * Created by Ilias Lamprou
 * Updated Oct 30 2016
 * 
 * Download latest Virtuino android app from the link:https://play.google.com/store/apps/details?id=com.virtuino_automations.virtuino&hl
 * Video tutorial link: https://youtu.be/4XEe0HY0j6k
 * Contact address for questions or comments: iliaslampr@gmail.com
 */

// Code to use SoftwareSerial
#include <SoftwareSerial.h>
#include "SoftwareSerial.h"
#include "SparkFunBME280.h"
SoftwareSerial espSerial =  SoftwareSerial(0,1);      // arduino RX pin=2  arduino TX pin=3    connect the arduino RX pin to esp8266 module TX pin   -  connect the arduino TX pin to esp8266 module RX pin
// BME280 
BME280 capteur;
// O2 code
#define Version     11          // version, 1.0 or 1.1, which depands on your board you use as it is
const int pinO2     = 15;       // Connect Grove - Gas Sensor(O2) to A0
 
#if Version==11
const int AMP   = 121;
#elif Version==10
const int AMP   = 201;
#endif
 
const float K_O2    = 7.43;
 
// CO2 code
SoftwareSerial K_30_Serial(7,8);  //Sets up a virtual serial port
                                    //Using pin 12 for Rx and pin 13 for Tx
byte readCO2[] = {0xFE, 0X44, 0X00, 0X08, 0X02, 0X9F, 0X25};  //Command packet to read Co2 (see app note)
byte response[] = {0,0,0,0,0,0,0};  //create an array to store the response
//multiplier for value. default is 1. set to 3 for K-30 3% and 10 for K-33 ICB
int valMultiplier = 1;


String apiKey = "YOPB6WIQNXLDDH0K";     // replace with your channel's thingspeak WRITE API key

//String ssid="WIFI NAME";    // Wifi network SSID
//String password ="WIFI PASSWORD";  // Wifi network password

boolean DEBUG=true;

//======================================================================== showResponce
void showResponse(int waitTime){
    long t=millis();
    char c;
    while (t+waitTime>millis()){
      if (espSerial.available()){
        c=espSerial.read();
        if (DEBUG) Serial.print(c);
      }
    }
                   
}

//========================================================================
boolean thingSpeakWrite(float value1, float value2, float value3, float value4, float value5){
  String cmd = "AT+CIPSTART=\"TCP\",\"";                  // TCP connection
  cmd += "184.106.153.149";                               // api.thingspeak.com
  cmd += "\",80";
  espSerial.println(cmd);
  if (DEBUG) Serial.println(cmd);
  if(espSerial.find("Error")){
    if (DEBUG) Serial.println("AT+CIPSTART error");
    return false;
  }
  
  
  String getStr = "GET /update?api_key=";   // prepare GET string
  getStr += apiKey;
  
  getStr +="&field1=";
  getStr += String(value1);
  getStr +="&field2=";
  getStr += String(value2);
  getStr +="&field3=";
  getStr += String(value3);
  getStr +="&field4=";
  getStr += String(value4);
  getStr +="&field5=";
  getStr += String(value5);
  // getStr +="&field3=";
  // getStr += String(value3);
  // ...
  getStr += "\r\n\r\n";

  // send data length
  cmd = "AT+CIPSEND=";
  cmd += String(getStr.length());
  espSerial.println(cmd);
  if (DEBUG)  Serial.println(cmd);
  
  delay(100);
  if(espSerial.find(">")){
    espSerial.print(getStr);
    if (DEBUG)  Serial.print(getStr);
  }
  else{
    espSerial.println("AT+CIPCLOSE");
    // alert user
    if (DEBUG)   Serial.println("AT+CIPCLOSE");
    return false;
  }
  return true;
}
//================================================================================ setup
void setup() {                
  DEBUG=true;           // enable debug serial
  Serial.begin(9600); 

  //BME280
 //while (!Serial) //{
    // Attente de l'ouverture du port série pour Arduino LEONARDO
  //}
  //configuration du capteur
  capteur.settings.commInterface = I2C_MODE; 
  capteur.settings.I2CAddress = 0x76;
  capteur.settings.runMode = 3; 
  capteur.settings.tStandby = 0;
  capteur.settings.filter = 0;
  capteur.settings.tempOverSample = 1 ;
  capteur.settings.pressOverSample = 1;
  capteur.settings.humidOverSample = 1;
  delay(10);  // attente de la mise en route du capteur. 2 ms minimum
  // chargement de la configuration du capteur
  capteur.begin();

 //CO2 code
  //Serial.begin(9600);         //Opens the main serial port to communicate with the computer
  K_30_Serial.begin(9600);    //Opens the virtual serial port with a baud of 9600

  
  espSerial.begin(9600);  // enable software serial
                          // Your esp8266 module's speed is probably at 115200. 
                          // For this reason the first time set the speed to 115200 or to your esp8266 configured speed 
                          // and upload. Then change to 9600 and upload again
  
  //espSerial.println("AT+RST");         // Enable this line to reset the module;
  //showResponse(1000);

  //espSerial.println("AT+UART_CUR=9600,8,1,0,0");    // Enable this line to set esp8266 serial speed to 9600 bps
  //showResponse(1000);
  
  

  //espSerial.println("AT+CWMODE=1");   // set esp8266 as client
  //showResponse(1000);

  //espSerial.println("AT+CWJAP=\""+ssid+"\",\""+password+"\"");  // set your home router SSID and password
  //showResponse(5000);

   //if (DEBUG)  Serial.println("Setup completed");
}


// ====================================================================== loop
void loop() {

// O2 code

   float sensorValue;
   float sensorVoltage;
   float Value_O2;
 
    sensorValue   = analogRead(15);
    sensorVoltage =((sensorValue/1024.0)*3.3)-1;
    sensorVoltage = sensorVoltage/(float)AMP*10000.0;
    Value_O2 = sensorVoltage/K_O2;
 
// CO2  code 
 sendRequest(readCO2);
 unsigned long valCO2 = getValue(response); 
  // Read sensor values
   float t = capteur.readTempC();  // value 1
   float h = capteur.readFloatHumidity(); // value 2
   float p = (capteur.readFloatPressure()*0.0075); // value 3
   float c = valCO2; // value 4
   float o = Value_O2; // value 5
   
   //float h = dht.readHumidity();
   
   thingSpeakWrite(t,h,c,o,p); 
        
 Serial.print("Température: ");
  Serial.print(capteur.readTempC(), 2);
  Serial.print(" °C");
  Serial.print("\t Pression: ");
  Serial.print((capteur.readFloatPressure()*0.0075), 2);
  Serial.print(" Pa");
  Serial.print("\t humidité relative : ");
  Serial.print(capteur.readFloatHumidity(), 2);
  Serial.println(" %");
  Serial.print("\t Co2 ppm = ");
  Serial.println(valCO2);
    
  // thingspeak needs 30 sec delay between updates,     
  delay(40000);  
}


//----------------

// CO2 sensor
void sendRequest(byte packet[])
{
  while(!K_30_Serial.available())  //keep sending request until we start to get a response
  {
    K_30_Serial.write(readCO2,7);
    delay(50);
  }
  
  int timeout=0;  //set a timeoute counter
  while(K_30_Serial.available() < 7 ) //Wait to get a 7 byte response
  {
    timeout++;  
    if(timeout > 10)    //if it takes to long there was probably an error
      {
        while(K_30_Serial.available())  //flush whatever we have
          K_30_Serial.read();
          
          break;                        //exit and try again
      }
      delay(50);
  }
  
  for (int i=0; i < 7; i++)
  {
    response[i] = K_30_Serial.read();
  }  
}

unsigned long getValue(byte packet[])
{
    int high = packet[3];                        //high byte for value is 4th byte in packet in the packet
    int low = packet[4];                         //low byte for value is 5th byte in the packet

  
    unsigned long val = high*256 + low;                //Combine high byte and low byte with this formula to get value
    return val* valMultiplier;
}

