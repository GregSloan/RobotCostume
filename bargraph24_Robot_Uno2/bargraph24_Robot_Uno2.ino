/*************************************************** 
  This is a library for our I2C LED Backpacks

  Designed specifically to work with the Adafruit LED 24 Bargraph Backpack
  ----> http://www.adafruit.com/products/721

  These displays use I2C to communicate, 2 pins are required to 
  interface. There are multiple selectable I2C addresses. For backpacks
  with 2 Address Select pins: 0x70, 0x71, 0x72 or 0x73. For backpacks
  with 3 Address Select pins: 0x70 thru 0x77

  Adafruit invests time and resources providing this open source code, 
  please support Adafruit and open-source hardware by purchasing 
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.  
  BSD license, all text above must be included in any redistribution
 ****************************************************/

#include <Wire.h>
#include "Adafruit_LEDBackpack.h"
#include "Adafruit_GFX.h"

Adafruit_24bargraph bar = Adafruit_24bargraph();
int led_pin = 13;
int potPin = A3;
int prev_pot_val = 0;
int cur_pot_val = 0;
int count = 0;
int refresh_buffer = 10;
int wait_count = 0;

void setup() {
  Serial.begin(9600);
  Serial.println("HT16K33 Bi-Color Bargraph test");
  pinMode(led_pin, OUTPUT);
  digitalWrite(led_pin, LOW);
  
  bar.begin(0x70);  // pass in the address

  for (uint8_t b=0; b<24; b++ ){
    if ((b % 3) == 0)  bar.setBar(b, LED_RED);
    if ((b % 3) == 1)  bar.setBar(b, LED_YELLOW);
    if ((b % 3) == 2)  bar.setBar(b, LED_GREEN);
  }
  bar.writeDisplay();
  delay(2000);
}


void loop() {
 
 cur_pot_val = analogRead(potPin);
 Serial.println("NEXT");
 Serial.println(prev_pot_val);
 Serial.println(cur_pot_val);

 if (cur_pot_val > 0 and cur_pot_val < 85) count = 12;
 if (cur_pot_val > 85 and cur_pot_val < 170) count = 11;
 if (cur_pot_val > 170 and cur_pot_val < 255) count = 10;
 if (cur_pot_val > 255 and cur_pot_val < 340) count = 9;
 if (cur_pot_val > 340 and cur_pot_val < 425) count = 8;
 if (cur_pot_val > 425 and cur_pot_val < 510) count = 7;
 if (cur_pot_val > 510 and cur_pot_val < 595) count = 6;
 if (cur_pot_val > 595 and cur_pot_val < 680) count = 5;
 if (cur_pot_val > 680 and cur_pot_val < 755) count = 4;
 if (cur_pot_val > 755 and cur_pot_val < 840) count = 3;
 if (cur_pot_val > 840 and cur_pot_val < 925) count = 2;
 if (cur_pot_val > 925) count = 1;
 Serial.println(count);
 for (uint8_t i=0; i < 24; i++) {

  if (i + 1 <= count*2) {
    if (i < 8){
      bar.setBar(i, LED_GREEN);

    }
    else if (i < 15) {
      bar.setBar(i, LED_YELLOW);

    }
    else {
      bar.setBar(i, LED_RED);

    }
  }

  else {
    bar.setBar(i, LED_OFF);
    
  }
 }
 wait_count ++;
 if (wait_count = refresh_buffer) {
 bar.writeDisplay();
 wait_count = 0;
 }
 
 prev_pot_val = cur_pot_val;
 /*****************
 if (cur_pot_val != prev_pot_val) {
   for (uint8_t i; i < count; i++) {
     digitalWrite(led_pin, HIGH);
     delay(1000);
     digitalWrite(led_pin, LOW);
     delay(1000);
     prev_pot_val = cur_pot_val;
   }
 }
 for (uint8_t b=0; b<24; b++) {
   bar.setBar(b, LED_RED);
   bar.writeDisplay();
   delay(50);
   bar.setBar(b, LED_OFF);
   bar.writeDisplay();
 }
  for (uint8_t b=0; b<24; b++) {
   bar.setBar(b, LED_GREEN);
   bar.writeDisplay();
   delay(50);
   bar.setBar(b, LED_OFF);
   bar.writeDisplay();
 }

 for (uint8_t b=0; b<24; b++) {
   bar.setBar(23-b, LED_YELLOW);
   bar.writeDisplay();
   delay(50);
   bar.setBar(23-b, LED_OFF);
   bar.writeDisplay();
 }
*/
}
