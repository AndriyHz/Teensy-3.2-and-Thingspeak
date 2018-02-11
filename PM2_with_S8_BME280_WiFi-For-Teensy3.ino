#include "SparkFunBME280.h"
// Code to use SoftwareSerial
#include "SoftwareSerial.h"
// CO2 code
SoftwareSerial K_30_Serial(0,1);  //Sets up a virtual serial port
                                    //Using pin 12 for Rx and pin 13 for Tx
byte readCO2[] = {0xFE, 0X44, 0X00, 0X08, 0X02, 0X9F, 0X25};  //Command packet to read Co2 (see app note)
byte response[] = {0,0,0,0,0,0,0};  //create an array to store the response
//multiplier for value. default is 1. set to 3 for K-30 3% and 10 for K-33 ICB
int valMultiplier = 1;
// Wi-Fi
SoftwareSerial espSerial =  SoftwareSerial(9,10);
// BME280 
BME280 capteur;

//Read and print serial data from Plantower PMS3003

uint16_t TPM01Value = 0;        //define PM1.0 variable
uint16_t TPM2_5Value = 0;       //define PM2.5 variable
uint16_t TPM10Value = 0;       //define PM10 variable
uint16_t PM01Value = 0;        //define PM1.0 variable
uint16_t PM2_5Value = 0;       //define PM2.5 variable
uint16_t PM10Value = 0;       //define PM10 variable

uint8_t receiveDat[24];       //define array to receive data from PMS3003
uint8_t testvalue;
int cnt = 0;
int passes = 1;

// Wi-Fi
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
        
// Plantower
  //  if (Serial1.available()>0) {
  //    testvalue=Serial1.read();
  if (Serial3.available() > 0) {
    testvalue = Serial3.read();
    if (testvalue == 0x42 && !(cnt == 0)) {
      cnt = 0;
    }
    //    if (testvalue == 66 && cnt)
    cnt = cnt + 1;

//            Serial.print(testvalue, HEX);
//            Serial.print(" ");
    receiveDat[cnt] = testvalue;   //receive 1 of 24 bytes from air detector module
    if(receiveDat[1] != 0x42 && receiveDat[2] != 0x42d){
      cnt = 0;
    }
    if (cnt == 24)
    {
      cnt = 0;
      //Serial.println("");
      TPM01Value = receiveDat[5] * 256 + receiveDat[6]; //calculate "TSI" PM1.0 concentration
      TPM2_5Value = receiveDat[7] * 256 + receiveDat[8]; //calculate "TSI" PM2.5 concentration
      TPM10Value = receiveDat[9] * 256 + receiveDat[10]; //calculate "TSI" PM10 concentration
     // Serial.print("{TSI 01 = ");
     // Serial.print(TPM01Value);
     // Serial.print(", TSI 2.5 = ");
     // Serial.print(TPM2_5Value);
     // Serial.print(", TSI 10 = ");
     // Serial.print(TPM10Value);
      PM01Value = receiveDat[11] * 256 + receiveDat[12]; //calculate PM1.0 concentration
      PM2_5Value = receiveDat[13] * 256 + receiveDat[14]; //calculate PM2.5 concentration
      PM10Value = receiveDat[15] * 256 + receiveDat[16]; //calculate PM10 concentration
 // CO2  code 
 sendRequest(readCO2);
 unsigned long valCO2 = getValue(response); 

///////////////////////////////////////////
/////// // Read sensor values ///////////////////////////
   float t = capteur.readTempC();  // value 1
   float h = capteur.readFloatHumidity(); // value 2
   float p = (capteur.readFloatPressure()*0.0075); // value 3
   float c = valCO2; // value 4
   // float o = Value_O2; // value 5
   float d = PM2_5Value; // value 7
   float e = PM10Value; // value 8
   //float h = dht.readHumidity();
   
   thingSpeakWrite(t,h,c,p,d,e); 
////////////////////////////////////////////////
////////// PRINT //////////////////////////////
      Serial.print("PM01 Value = ");
      Serial.print(PM01Value);
      Serial.println("  ug/m3");
      Serial.print("PM2.5 Value = ");
      Serial.print(PM2_5Value);
      Serial.println("  ug/m3");  
      Serial.print("PM10 Value = ");
      Serial.print(PM10Value);
      Serial.println("  ug/m3");
      Serial.println();   
      delay(4000); 
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

