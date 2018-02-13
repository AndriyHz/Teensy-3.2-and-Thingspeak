#include "SparkFunBME280.h"
// Code to use SoftwareSerial
#include "SoftwareSerial.h"
////////////////////////////////
// Plantower
///////////////////////////////
#include "PMS.h"
PMS pms(Serial3);
PMS::DATA data;
//////////////////////////////
// CO2 code
/////////////////////////////
SoftwareSerial K_30_Serial(0,1);  //Sets up a virtual serial port
                                    //Using pin 12 for Rx and pin 13 for Tx
byte readCO2[] = {0xFE, 0X44, 0X00, 0X08, 0X02, 0X9F, 0X25};  //Command packet to read Co2 (see app note)
byte response[] = {0,0,0,0,0,0,0};  //create an array to store the response
//multiplier for value. default is 1. set to 3 for K-30 3% and 10 for K-33 ICB
int valMultiplier = 1;
///////////////////////////////////
// Wi-Fi
////////////////////////////////////
SoftwareSerial espSerial =  SoftwareSerial(9,10);
// BME280 
BME280 capteur;

///////////////////////////////////
// Wi-Fi
//////////////////////////////////
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
boolean thingSpeakWrite(float value1, float value2, float value3, float value4, float value7, float value8){
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
  //getStr +="&field5=";
  //getStr += String(value5);
  getStr +="&field7=";
  getStr += String(value7);
  getStr +="&field8=";
  getStr += String(value8);
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

void setup() {
 DEBUG=true;           // enable debug serial
  Serial.begin(9600); 
  
 //Serial.begin(9600);   //set the serial's Baudrate of the air detector module
  Serial3.begin(9600);
 //CO2 code
  K_30_Serial.begin(9600);    //Opens the virtual serial port with a baud of 9600

 //BME280
 //while (!Serial) {
    // Attente de l'ouverture du port série pour Arduino LEONARDO
 // }
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
  ///////////////////////////////////
// espSerial //
///////////////////////////////////
espSerial.begin(9600);
  
  }

void loop()
{
//////////////////////////////////        
/// Plantower //////
//////////////////////////////////
 
   if (pms.read(data))
  {
    Serial.println("Data:");

    Serial.print("PM 1.0 (ug/m3): ");
    Serial.println(data.PM_AE_UG_1_0);

    Serial.print("PM 2.5 (ug/m3): ");
    Serial.println(data.PM_AE_UG_2_5);

    Serial.print("PM 10.0 (ug/m3): ");
    Serial.println(data.PM_AE_UG_10_0);

    Serial.println();
    {
      
     
 ////////////////////////////
 //// CO2  code ///////
 ///////////////////////////
 sendRequest(readCO2);
 unsigned long valCO2 = getValue(response); 

///////////////////////////////////////////
/////// // Read sensor values ///////////////////////////
   float t = capteur.readTempC();  // value 1
   float h = capteur.readFloatHumidity(); // value 2
   float p = (capteur.readFloatPressure()*0.0075); // value 3
   float c = valCO2; // value 4
   // float o = Value_O2; // value 5
   float d = data.PM_AE_UG_2_5; // value 7
   float e = data.PM_AE_UG_10_0; // value 8
   //float h = dht.readHumidity();
   
   thingSpeakWrite(t,h,c,p,d,e);

 
////////////////////////////////////////////////
////////// PRINT //////////////////////////////
  
      Serial.print("Température: ");
      Serial.print(capteur.readTempC(), 2);
      Serial.print("°C");
      Serial.print("\t Pression: ");
      Serial.print((capteur.readFloatPressure()*0.0075), 2);
      Serial.print(" Pa");
      Serial.print("\t Humidité relative : ");
      Serial.print(capteur.readFloatHumidity(), 2);
      Serial.print(" %");
      Serial.print("\t Co2 ppm = ");
      Serial.println(valCO2);
      Serial.println("\t =============== ");
      delay(4000);
    }
 }
 
}
///////////////////////////////
// CO2 sensor //////
///////////////////////////////
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

