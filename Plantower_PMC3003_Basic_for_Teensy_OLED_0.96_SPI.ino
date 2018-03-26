#include "PMS.h"
PMS pms(Serial3);
PMS::DATA data;

/// OLED ////////////
#include <TeensyView.h>  // Include the SFE_TeensyView library
#include "Wire.h"
#include "SPI.h"
///////////////////////////////////
// TeensyView Object Declaration //
///////////////////////////////////
//Standard
#define PIN_RESET 14
#define PIN_DC    9
#define PIN_CS    10
#define PIN_SCK   13
#define PIN_MOSI  11
TeensyView oled(PIN_RESET, PIN_DC, PIN_CS, PIN_SCK, PIN_MOSI);

void setup()
{
  Serial.begin(9600);   // GPIO1, GPIO3 (TX/RX pin on ESP-12E Development Board)
  Serial3.begin(9600);  // GPIO2 (D4 pin on ESP-12E Development Board)
  ///////////////////////////////////
// TeensyView OLED //
///////////////////////////////////
  oled.begin();    // Initialize the OLED
  oled.clear(ALL); // Clear the display's internal memory
  oled.display();  // Display what's in the buffer (splashscreen)
  delay(1000);     // Delay 1000 ms
  oled.clear(PAGE); // Clear the buffer.
}

void loop()
{
   ///////////////////////////////////
// TeensyView OLED //
/////////////////////////////////// 
  oled.clear(PAGE);  // Clear the page
  
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
  ///////////////////////////////////
// TeensyView OLED //
///////////////////////////////////
 // PM2.5
  oled.setFontType(0);  // Set font to type 0
  oled.setCursor(90, 0); // move cursor
  oled.print("PM2.5:");  // Write a byte out as a character
  oled.setFontType(0);  // Set font to type 0
  oled.setCursor(90, 8); // move cursor
  oled.print(data.PM_AE_UG_2_5, 1);  // Write a byte out as a param
  
  // PM10
  oled.setFontType(0);  // Set font to type 0
  oled.setCursor(90, 17); // move cursor
  oled.print("PM10:");  // Write a byte out as a character
  oled.setFontType(0);  // Set font to type 0
  oled.setCursor(90, 25); // move cursor
  oled.print(data.PM_AE_UG_10_0, 1);  // Write a byte out as a param
  oled.display();  // Send the PAGE to the OLED memory
  
////////////////////////////////////////////////
    
  }
}
