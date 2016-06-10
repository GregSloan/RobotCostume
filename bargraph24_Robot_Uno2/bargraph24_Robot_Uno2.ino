/***************************************************
  This is an effects control program for a robot costume

  Designed specifically to work with the Adafruit LED 24 Bargraph Backpack
  ----> http://www.adafruit.com/products/721

  FEATURES:
  * Various bargraph display patterns, selectable by input button
  * A "volume level" type indicator on the bar graph controlled by potentiometer
    * interrupts bagraph display pattern, then returns to pattern after idle
    * Sends I2C signal to seprate Uno with Wave shield to trigger sound effects on level changes
  * 10 LED array, 2 symmetric "star" layouts with button-selectable patterns
  * Sound effects triggered by multiple buttons on a single Analog pin
  * IR break beam triggered sound effect
 ****************************************************/

#include <Wire.h>
#include "Adafruit_LEDBackpack.h"
#include "Adafruit_GFX.h"


const int led_pin = 13; //On-board LED

//************************************************
//Sound Effects Buttons Declarations
//************************************************
uint8_t audio_buttons = A2;
uint8_t audio_button_hold = false;
unsigned long cycle_time = 0;

//************************************************


//************************************************
//LED Star Array Declarations
//************************************************
const uint8_t star00 = A1;           //pin set 1
const uint8_t star01 = A0;
const uint8_t star02 = 3;
const uint8_t star03 = 4;
const uint8_t star04 = 5;

const uint8_t star10 = 8;           //pin set 2
const uint8_t star11 = 9;
const uint8_t star12 = 10;
const uint8_t star13 = 11;
const uint8_t star14 = 12;

unsigned long star_cycle_time = 0;  //Holds millis of last cycle action on the star array
const uint8_t starCtrl = 6;         //Pin for star pattern selector button
uint8_t starCtrlState = 0;          //To hold read-in state of the button
uint8_t starCtrlPrevState = 0;      //To hold previously recorede state of the button

bool starCtrlInit = true;           //Flag for whether the star pattern should be initialized this cycle

uint8_t star_selector = 1;          //Currently selected pattern
uint8_t star_mode_count = 3;        //Total number of defined patterns
//*************************************************


//*************************************************
//Bar Graph Declarations
//*************************************************
Adafruit_24bargraph bar = Adafruit_24bargraph();  //Bargraph control object
const int pot_pin = A3;                           //Potentiometer input pin
int prev_pot_val = 0;                             //Potentiometer value at last reading
int cur_pot_val = 0;                              //To hold new readings of pot
int level_count = 0;                              //Number of levels to display on the bar graph based on pot input
const int refresh_buffer = 3000;                  //Number of cycles to hold bar graph level display before returning to pattern
int wait_count = 0;                               //Number of cycles since bar graph level began display
float change_rate = .05;                          //Tolerance for the potentiometer reading
int color_bar_refresh = 500;                      //Refresh rate for the "Color_bars" pattern
unsigned long color_bar_cycle_time = 0;           //Time since last refresh of Color_bars pattern
int pulse_dir = 1;                                //Direction flag for the solid_pulse pattern
int last_led = 0;                                 //Most recently modified bargraph LED
int last_color = LED_RED;                         //Track the last color used by bargraph pattern
const int bar_button_pin = 2;                         //Input pin for pattern selector button
int bar_button_state = 0;                             //Holds currently read button state
int bar_button_prev = 0;                              //Previously recorded button state
int bar_selector = 0;                             //Currently selected bar mode
int bar_mode_count = 3;                           //Total number of available bar modes
//*************************************************



//*************************************************
//Beam break declarations
//*************************************************
const int beam_pin = 7;       //Input pin
uint8_t last_beam_state = 0;  //Previously recorded state
//*************************************************


//!!!!!!!!!!!!!!!!!!!!!!!
//Begin Star Mode Methods
//!!!!!!!!!!!!!!!!!!!!!!!

//***************************************
//star_random_flash
//Randomly turn LEDs on and off
//Selector Index: 2
//***************************************
void star_random_flash() {
  uint8_t all_LEDs[] = {star00, star01, star02, star03, star04, star10, star11, star12, star13, star14};
  if (starCtrlInit) {
    Serial.println(sizeof(all_LEDs)/sizeof(uint8_t));
    randomSeed(millis());
    starCtrlInit = false;
    star_cycle_time = millis();
    for (int led = 0; led < (sizeof(all_LEDs)/sizeof(uint8_t)); led++) {
      random_led_state(all_LEDs[led]);
    }
  }
  
  if (millis() - star_cycle_time > 300) {
    for (int led = 0; led < (sizeof(all_LEDs)/sizeof(uint8_t)); led++) {
      random_led_state(all_LEDs[led]);
      star_cycle_time = millis();
    }
  }
}
//****************
//Utility funciton to randomly set an LED state
//****************
void random_led_state(uint8_t pin) {
  long rNum = random(100);
  if (rNum < 50) digitalWrite(pin, LOW);
  else digitalWrite(pin, HIGH);
  
}

//***************************************
//star_symmetric_chase
//Turn the pins on in a "chase" order
//Same order and timing each side
//Selector Index: 1
//***************************************
void star_symmetric_chase() {
  if (starCtrlInit) {
    digitalWrite(star00, HIGH);
    digitalWrite(star01, LOW);
    digitalWrite(star02, LOW);
    digitalWrite(star03, LOW);
    digitalWrite(star04, LOW);
    digitalWrite(star14, HIGH);
    digitalWrite(star13, LOW);
    digitalWrite(star12, LOW);
    digitalWrite(star11, LOW);
    digitalWrite(star10, LOW);
    star_cycle_time = millis();
    starCtrlInit = false;
  }

  if (millis() - star_cycle_time > 200) {
    if (digitalRead(star00) == HIGH) {
      digitalWrite(star00, LOW);
      digitalWrite(star14, LOW);
      digitalWrite(star01, HIGH);
      digitalWrite(star13, HIGH);

    }
    else if (digitalRead(star01) == HIGH) {
      digitalWrite(star01, LOW);
      digitalWrite(star13, LOW);
      digitalWrite(star02, HIGH);
      digitalWrite(star12, HIGH);
    }
    else if (digitalRead(star02) == HIGH) {
      digitalWrite(star02, LOW);
      digitalWrite(star12, LOW);
      digitalWrite(star03, HIGH);
      digitalWrite(star11, HIGH);
    }
    else if (digitalRead(star03) == HIGH) {
      digitalWrite(star03, LOW);
      digitalWrite(star11, LOW);
      digitalWrite(star04, HIGH);
      digitalWrite(star10, HIGH);
    }
    else {
      digitalWrite(star04, LOW);
      digitalWrite(star10, LOW);
      digitalWrite(star00, HIGH);
      digitalWrite(star14, HIGH);
    }
    star_cycle_time = millis();
  }
}
//*****************************************


//****************************************
//star_alternate_blink
//Alternately blinks all LEDs on one side 
//Selector Index: 0
//***************************************
void star_alternate_blink() {

  if (starCtrlInit) {
    Serial.println("Star alternate blink Init");
    digitalWrite(star00, HIGH);
    digitalWrite(star01, HIGH);
    digitalWrite(star02, HIGH);
    digitalWrite(star03, HIGH);
    digitalWrite(star04, HIGH);
    digitalWrite(star10, LOW);
    digitalWrite(star11, LOW);
    digitalWrite(star12, LOW);
    digitalWrite(star13, LOW);
    digitalWrite(star14, LOW);
    star_cycle_time = millis();
    starCtrlInit = false;
  }

  int leftState = LOW;
  int rightState = HIGH;
  if (millis() - star_cycle_time > 1000) {
    if (digitalRead(star00) == LOW) {
      leftState = HIGH;
      rightState = LOW;
    }
    digitalWrite(star00, leftState);
    digitalWrite(star01, leftState);
    digitalWrite(star02, leftState);
    digitalWrite(star03, leftState);
    digitalWrite(star04, leftState);
    digitalWrite(star10, rightState);
    digitalWrite(star11, rightState);
    digitalWrite(star12, rightState);
    digitalWrite(star13, rightState);
    digitalWrite(star14, rightState);

    star_cycle_time = millis();

  }
}
//***************************************************


//!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//Begin Bargraph Mode Methods
//!!!!!!!!!!!!!!!!!!!!!!!!!!!


//*******************************************
//solid_pulse
//Solid backgroud color
//contrasting single led travels
//Goes both directions, then background and pulse color change
//Selector Index: 2
//*******************************************
void solid_pulse() {
  int pulse_color = 0;
  if (last_color == LED_RED) pulse_color = LED_GREEN;
  if (last_color == LED_YELLOW) pulse_color = LED_RED;
  if (last_color == LED_GREEN) pulse_color = LED_YELLOW;


  if (last_led == 0) {
    pulse_dir = 1;
    if (last_color == LED_RED) {
      last_color = LED_YELLOW;
      pulse_color = LED_RED;
    }
    else if (last_color == LED_YELLOW) {
      last_color = LED_GREEN;
      pulse_color = LED_YELLOW;
    }
    else {
      last_color = LED_RED;
      pulse_color = LED_GREEN;
    }


  }
  else if (last_led == 23) {
    pulse_dir = -1;
  }


  for (int i = 0; i < 24; i++) {
    bar.setBar(i, last_color);

  }
  bar.setBar(last_led, pulse_color);
  bar.writeDisplay();
  delay(50);
  bar.setBar(last_led, last_color);
  bar.writeDisplay();
  last_led += pulse_dir;
}



//****************************************
//color_bars
//Cycles through the 3 colors
//Turns the whole bar that color each cycle
//Selector Index: 1
//*****************************************
void color_bars() {
  unsigned long current_time = millis();

  if (current_time - color_bar_cycle_time > color_bar_refresh) {
    if (last_color == LED_RED) last_color = LED_YELLOW;
    else if (last_color == LED_YELLOW) last_color = LED_GREEN;
    else last_color = LED_RED;
    for (int i = 0; i < 24; i++) {
      bar.setBar(i, last_color);
    }
    bar.writeDisplay();
    color_bar_cycle_time = millis();
  }
}

//****************************************
//cylon
//Single LED scans across, no background color
//Color changes after each time across
//Selector Index: 0
//*****************************************
void cylon() {
  if (last_led < 24) {
    last_led++;
  }
  else {
    last_led = 0;
    if (last_color == LED_RED) last_color = LED_YELLOW;
    else if (last_color == LED_YELLOW) last_color = LED_GREEN;
    else last_color = LED_RED;
  }

  bar.setBar(23 - last_led, last_color);
  bar.writeDisplay();
  delay(50);
  bar.setBar(23 - last_led, LED_OFF);
  bar.writeDisplay();

}


//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//Begin Operation
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!


void setup() {
  //Wire.begin();
  pinMode(audio_buttons, INPUT_PULLUP);               //Analog input for multi-button sound effect triggers

  Serial.begin(9600);
  Serial.println("HT16K33 Bi-Color Bargraph test");
  
  pinMode(led_pin, OUTPUT);                           //On-board LED
  digitalWrite(led_pin, LOW);                         //Start LED off
  pinMode(bar_button_pin, INPUT);                     //Bar mode selector button input pin
  pinMode(beam_pin, INPUT);                           //IR beam break input pin
  digitalWrite(beam_pin, HIGH);                       //Start high

  pinMode(starCtrl, INPUT);                           //Star array pattern selector button input
  pinMode(star00, OUTPUT);                            //Set all LED output pins for the star array
  pinMode(star01, OUTPUT);
  pinMode(star02, OUTPUT);
  pinMode(star03, OUTPUT);
  pinMode(star04, OUTPUT);
  pinMode(star10, OUTPUT);
  pinMode(star11, OUTPUT);
  pinMode(star12, OUTPUT);
  pinMode(star13, OUTPUT);
  pinMode(star14, OUTPUT);

  //Test pattern for star array
  //All On
  digitalWrite(star00, HIGH);
  digitalWrite(star01, HIGH);
  digitalWrite(star02, HIGH);
  digitalWrite(star03, HIGH);
  digitalWrite(star04, HIGH);
  digitalWrite(star10, HIGH);
  digitalWrite(star11, HIGH);
  digitalWrite(star12, HIGH);
  digitalWrite(star13, HIGH);
  digitalWrite(star14, HIGH);

  delay(500);
  //All Off
  digitalWrite(star00, LOW);
  digitalWrite(star01, LOW);
  digitalWrite(star02, LOW);
  digitalWrite(star04, LOW);
  digitalWrite(star03, LOW);
  digitalWrite(star10, LOW);
  digitalWrite(star11, LOW);
  digitalWrite(star12, LOW);
  digitalWrite(star13, LOW);
  digitalWrite(star14, LOW);

  delay(500);
  //One side on
  digitalWrite(star00, HIGH);
  digitalWrite(star01, HIGH);
  digitalWrite(star02, HIGH);
  digitalWrite(star03, HIGH);
  digitalWrite(star04, HIGH);
  star_cycle_time = millis();

  //Initialize bargraph on the I2C bus
  bar.begin(0x70);  // pass in the address

  //Test pattern for bargraph
  for (uint8_t b = 0; b < 24; b++ ) {
    if ((b % 3) == 0)  bar.setBar(b, LED_RED);
    if ((b % 3) == 1)  bar.setBar(b, LED_YELLOW);
    if ((b % 3) == 2)  bar.setBar(b, LED_GREEN);
  }
  bar.writeDisplay();
  delay(2000);
}

void loop() {
  //************************************************
  // Audio effects buttons control
  //**************************************************
  
  int audio_button_average = 0;
  for (int i = 0; i < 10; i++) audio_button_average += analogRead(audio_buttons);
  
  audio_button_average /= 10;

  if (millis() - cycle_time > 1000){
  Serial.println(audio_button_average);
  Serial.println(audio_button_hold);
  cycle_time = millis();
  }
  if (audio_button_average < 50) {
    if (audio_button_hold == false){
    Wire.beginTransmission(0x8);
    Wire.write(14);
    Wire.endTransmission();
    Serial.println("Sending Audio Control Code: 14");
    Serial.println(audio_button_average);
    audio_button_hold = true;
    }
  }
  else if ((65 < audio_button_average) && (audio_button_average < 100) ) {
    if (audio_button_hold == false){
    Wire.beginTransmission(0x8);
    Wire.write(15);
    Wire.endTransmission();
    Serial.println("Sending Audio Control Code: 15");
    Serial.println(audio_button_average);
    audio_button_hold = true;
    }
  }
  else if ((105 < audio_button_average) && (audio_button_average  < 155)) {
    if (audio_button_hold == false){
    Wire.beginTransmission(0x8);
    Wire.write(16);
    Wire.endTransmission();
    Serial.println("Sending Audio Control Code: 16");
    Serial.println(audio_button_average);
    audio_button_hold = true;
    }
  }
  else if ((165 < audio_button_average) && (audio_button_average < 205)) {
    if (audio_button_hold == false){
    Wire.beginTransmission(0x8);
    Wire.write(17);
    Wire.endTransmission();
    Serial.println("Sending Audio Control Code: 17");
    Serial.println(audio_button_average);
    audio_button_hold = true;
    }    
  }
  else {
    audio_button_hold = false;
  }
  
  //****************************************


  //************************************
  //Handle IR beam to trigger candy thank you audio
  //***************************************
  int beam_state = digitalRead(beam_pin);

  if (beam_state == LOW) {
    digitalWrite(led_pin, HIGH);      //Debug, turn on on-borad LED when beam is broken
    if (last_beam_state == HIGH) {
      //Transmit 13 over I2C to Wave shield at 0x8
      Wire.beginTransmission(0x8);
      Wire.write(13);
      Wire.endTransmission();
      Serial.println("Beam broken");

    }
    last_beam_state = LOW;
  }
  else {
    digitalWrite(led_pin, LOW);
    last_beam_state = HIGH;

  }
  //*********************************************


  //********************************************
  //Use potentiometer to control bargraph "level" display
  //***********************************************
  cur_pot_val = 0;
  
  for (int i = 0; i < 6; i++) cur_pot_val += analogRead(pot_pin);   //Get average of 6 readings
  cur_pot_val /= 6;

  
  //Set change sensetivity based on the current level
  //Necessary because there is more flucuation at lower values
  //***********************************************
  if (cur_pot_val == 0) change_rate = 1;
  else if (cur_pot_val < 40) change_rate = .75;
  else if (cur_pot_val < 100) change_rate = .2;
  if (cur_pot_val < 200) change_rate = .1;
  else if (cur_pot_val > 1000) change_rate = .015;
  else change_rate = .05;

  //If potentiometer value has changed beyond the change_rate, update display with level bars based on the reading
  //Also triggers level-determined audio
  if (cur_pot_val < prev_pot_val * (1.0 - change_rate) or cur_pot_val > prev_pot_val * (1.0 + change_rate)) {
    if (cur_pot_val >= 0 and cur_pot_val < 85) level_count = 1;
    if (cur_pot_val >= 85 and cur_pot_val < 170) level_count = 2;
    if (cur_pot_val >= 170 and cur_pot_val < 255) level_count = 3;
    if (cur_pot_val >= 255 and cur_pot_val < 341) level_count = 4;
    if (cur_pot_val >= 341 and cur_pot_val < 426) level_count = 5;
    if (cur_pot_val >= 426 and cur_pot_val < 511) level_count = 6;
    if (cur_pot_val >= 511 and cur_pot_val < 596) level_count = 7;
    if (cur_pot_val >= 596 and cur_pot_val < 687) level_count = 8;
    if (cur_pot_val >= 687 and cur_pot_val < 787) level_count = 9;
    if (cur_pot_val >= 787 and cur_pot_val < 880) level_count = 10;
    if (cur_pot_val >= 880 and cur_pot_val < 975) level_count = 11;
    if (cur_pot_val >= 975) level_count = 12;
    
    Wire.beginTransmission(0x8);

    Wire.write(level_count);

    Wire.endTransmission();
    delay(100);

    for (uint8_t i = 0; i < 24; i++) {
      if (i + 1 <= level_count * 2) {
        if (i < 8) {
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

  //If pot vlaue is steady for refresh_buffer cycles, go back to currently selected pattern
  else if (wait_count == refresh_buffer) {
    if (bar_selector == 0) cylon();
    else if (bar_selector == 1) color_bars();
    else if (bar_selector == 2) solid_pulse();

  }
  else {
    wait_count++;
  }
  //***********************************************

  //****************************************
  //Select bargraph pattern with button press
  //******************************************
  bar_button_state = digitalRead(bar_button_pin);

  if (bar_button_state == HIGH and bar_button_prev == LOW) {
    //Cycle back to 0 if it's the last pattern
    if (bar_selector == bar_mode_count - 1) bar_selector = 0;
    else bar_selector += 1;

    bar_button_prev = HIGH;
  }
  else if (bar_button_state == LOW)
  {

    bar_button_prev = LOW;
  }
  
  //*************************************************
  //Execute a star pattern based on current selector
  //*************************************************
  if (star_selector == 0) star_alternate_blink();
  if (star_selector == 1) star_symmetric_chase();
  if (star_selector == 2) star_random_flash();

  //Select star pattern with button press  
  starCtrlState = digitalRead(starCtrl);
  //If pressed, change the selected pattern and turn in Init flag
  if (starCtrlState == HIGH and starCtrlPrevState == LOW) {
    if (star_selector == star_mode_count - 1) star_selector = 0;
    else star_selector += 1;
    starCtrlInit = true;
    starCtrlPrevState = HIGH;
  }
  else if (starCtrlState == LOW)
  {
    starCtrlPrevState = LOW;
  }


}




