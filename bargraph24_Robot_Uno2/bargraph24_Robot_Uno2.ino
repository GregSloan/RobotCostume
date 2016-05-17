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
const int led_pin = 13;
const int pot_pin = A3;
int prev_pot_val = 0;
int cur_pot_val = 0;
int count = 0;
const int refresh_buffer = 10000;
int wait_count = 0;
float change_rate = .05;
int color_bar_refresh = 500;
unsigned long color_bar_cycle_time = 0;

const int button_pin = 2;
int button_state = 0;
int button_prev = 0;

int last_led = 0;
int last_color = LED_RED;

int led_selector = 0;
int led_mode_count = 2;

void setup() {
  Serial.begin(9600);
  Serial.println("HT16K33 Bi-Color Bargraph test");
  pinMode(led_pin, OUTPUT);
  digitalWrite(led_pin, LOW);
  pinMode(button_pin, INPUT);
  
  bar.begin(0x70);  // pass in the address

  for (uint8_t b=0; b<24; b++ ){
    if ((b % 3) == 0)  bar.setBar(b, LED_RED);
    if ((b % 3) == 1)  bar.setBar(b, LED_YELLOW);
    if ((b % 3) == 2)  bar.setBar(b, LED_GREEN);
  }
  bar.writeDisplay();
  delay(2000);
}

void color_bars(){
  unsigned long current_time = millis();

  if (current_time - color_bar_cycle_time > color_bar_refresh) {
  if (last_color == LED_RED) last_color = LED_YELLOW;
   else if (last_color == LED_YELLOW) last_color = LED_GREEN;
   else last_color = LED_RED;
  for (int i=0; i < 24; i++){
    bar.setBar(i, last_color);
  }
  bar.writeDisplay();
  color_bar_cycle_time = millis();
  }  
}

void cylon(){
  if (last_led < 24){
    last_led++;
  }
  else {
    last_led = 0;
    if (last_color == LED_RED) last_color = LED_YELLOW;
    else if (last_color == LED_YELLOW) last_color = LED_GREEN;
    else last_color = LED_RED;
  }
    
    bar.setBar(23-last_led,last_color);
    bar.writeDisplay();
    delay(50);
    bar.setBar(23-last_led,LED_OFF);
    bar.writeDisplay();

}

void loop() {
 cur_pot_val = 0;
 for (int i=0; i < 6; i++) cur_pot_val += analogRead(pot_pin);

 cur_pot_val /= 6;
 /*Serial.println("NEXT");
 Serial.println(prev_pot_val);
 Serial.println(cur_pot_val);*/
 if (cur_pot_val == 0) change_rate = 1;
 else if (cur_pot_val < 40) change_rate = .75;
 else if (cur_pot_val < 100) change_rate = .2;
 if (cur_pot_val < 200) change_rate = .1;
 else if (cur_pot_val > 1000) change_rate = .015;
 else change_rate = .05;
 
 if (cur_pot_val < prev_pot_val*(1.0-change_rate) or cur_pot_val > prev_pot_val*(1.0+change_rate)){
  Serial.println(prev_pot_val*(1.0-change_rate));
  Serial.println(prev_pot_val*(1.0+change_rate));
  Serial.println(cur_pot_val);
  Serial.println(prev_pot_val);
  Serial.println("-----");
 if (cur_pot_val >= 0 and cur_pot_val < 85) count = 1;
 if (cur_pot_val >= 85 and cur_pot_val < 170) count = 2;
 if (cur_pot_val >= 170 and cur_pot_val < 255) count = 3;
 if (cur_pot_val >= 255 and cur_pot_val < 341) count = 4;
 if (cur_pot_val >= 341 and cur_pot_val < 426) count = 5;
 if (cur_pot_val >= 426 and cur_pot_val < 511) count = 6;
 if (cur_pot_val >= 511 and cur_pot_val < 596) count = 7;
 if (cur_pot_val >= 596 and cur_pot_val < 687) count = 8;
 if (cur_pot_val >= 687 and cur_pot_val < 787) count = 9;
 if (cur_pot_val >= 787 and cur_pot_val < 880) count = 10;
 if (cur_pot_val >= 880 and cur_pot_val < 975) count = 11;
 if (cur_pot_val >= 975) count = 12;
 /*Serial.println(count);*/
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
 bar.writeDisplay();
 wait_count = 0;
 
 
 prev_pot_val = cur_pot_val;
 }

else if (wait_count == refresh_buffer) {
  if (led_selector == 0) cylon();
  else if (led_selector == 1) color_bars();
  /*
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
 }*/
}

else {
  wait_count++;
}
 
 button_state = digitalRead(button_pin);


 if (button_state == HIGH and button_prev == LOW){
  /*Serial.println("In");
  Serial.println(digitalRead(led_pin));*/
  if (led_selector == led_mode_count -1) led_selector = 0;
  else led_selector += 1;
  
  if (digitalRead(led_pin) == HIGH){
    digitalWrite(led_pin, LOW);
  }
  else {
    digitalWrite(led_pin, HIGH);
    
  }
  button_prev = HIGH;
  }
  else if (button_state == LOW) 
  {

    button_prev = LOW;
  }
  
  
 }



