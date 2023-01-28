//INCLUDE TM1637 4-DIGIT DISPLAY MODULE LIBRARY - FOUND @ https://github.com/AKJ7/TM1637
#include <TM1637.h>

// Instantiation and pins configurations
// Pin 4 - > CLK
// Pin 5 - > DIO
TM1637 tm(4, 5);

#define  ENCODER_OPTIMIZE_INTERRUPTS
#include <Wire.h>
#include <Encoder.h>

Encoder myEnc(3, 2);
float oldPosition1  = -999;
float newPosition1  = -999;
float oldPosition2  = -999;
float newPosition2  = -999;
float oldPosition3  = -999;
float newPosition3  = -999;

byte enc_switch_in = 0;
unsigned long enc_switch_timer = 0;
bool enc_switch_latch = false;
byte enc_switch_counter = 0;
byte enc_switch = 0;
byte toggle_switch = 0;//0=EXT,1=ON
static int DOUBLE_CLICK_TIME = 500;

float BPM = 125.0;
float delayPerBeat = 0.0;
float msPerMin = 1000.0 * 60.0; //1000ms x 60s = 60,000ms per min
float bpmDelay = 0.0;

unsigned long currentTime = 0;
unsigned long previousTime = 0;        // will store last time LED was updated
unsigned long counter = 0;        // will store last time LED was updated

const int clockLed1 = 9;
const int clockLed2 = 8;
const int clockLed3 = 6;
const int clockLed4 = 7;
const int clockLed5 = 11;
const int clockLed6 = 12;
const int clockLed7 = 18;
const int clockLed8 = 19;

bool clockState1 = LOW;             // clockState1 used to set the LED
bool clockState2 = LOW;             // clockState1 used to set the LED
bool clockState3 = LOW;             // clockState1 used to set the LED
bool clockState4 = LOW;             // clockState1 used to set the LED
bool clockState5 = LOW;             // clockState1 used to set the LED
bool clockState6 = LOW;             // clockState1 used to set the LED
bool clockState7 = LOW;             // clockState1 used to set the LED
bool clockState8 = LOW;             // clockState1 used to set the LED

byte prog = 0;

bool modeSwitch = 0;
byte mode = 0;

bool pulseSwitchIn = 0;

const static int pgm[24][7] = {
  {2, 4, 8, 16, 32, 64, 128},          {3, 5, 7, 9, 11, 13, 15},        {2, 3, 4, 5, 6, 7, 8},                     {3, 5, 8, 13, 21, 34, 55},
  {2, 3, 5, 7, 11, 13, 17},            {3, 6, 10, 15, 21, 28, 36},      {4, 9, 16, 25, 36, 49, 64},                {4, 10, 20, 35, 56, 84, 120},
  {5, 14, 30, 55, 91, 140, 204},       {8, 27, 64, 125, 216, 343, 512}, {32, 243, 1024, 3125, 7776, 16807, 32768}, {13, 37, 73, 121, 181, 253, 337},
  {14, 51, 124, 245, 426, 679, 1016},  {2, 4, 8, 12, 24, 48, 72},       {16, 22, 34, 36, 46, 56, 64},              {72, 108, 200, 288, 392, 432, 500},
  {6, 21, 28, 301, 325, 496, 697},     {2, 8, 20, 28, 50, 82, 126},     {21, 33, 57, 69, 77, 93, 129},             {2, 3, 2, 5, 6, 7, 2},
  {30, 42, 66, 70, 78, 102, 105},      {9, 45, 55, 99, 297, 703, 999},  {70, 836, 4030, 5830, 7192, 7912, 9272},   {15, 34, 65, 111, 175, 260, 369},
};

void setup() {

  tm.begin();
  tm.setBrightness(4);

  pinMode(10, INPUT_PULLUP); //Encoder Button

  pinMode(clockLed1, OUTPUT);
  pinMode(clockLed2, OUTPUT);
  pinMode(clockLed3, OUTPUT);
  pinMode(clockLed4, OUTPUT);
  pinMode(clockLed5, OUTPUT);
  pinMode(clockLed6, OUTPUT);
  pinMode(clockLed7, OUTPUT);
  pinMode(clockLed8, OUTPUT);

  pinMode(14, INPUT_PULLUP); //MODE SWITCH
  pinMode(15, INPUT_PULLUP); //PULSE SWITCH

}


void loop() {
  hardwareCheck();
  segDisplay();
  clockOut();
  resetcntr();
}


void hardwareCheck() {

  enc_switch_in  = !digitalRead(10);

  if ((enc_switch_in == true) && (enc_switch_latch == false)) {  //Count Times Button Pressed
    enc_switch_counter++;
    enc_switch_timer = millis();
    enc_switch_latch = true;
  }
  else if (enc_switch_in == false) {
    enc_switch_latch = false;
  }

  if ((millis() >= enc_switch_timer + DOUBLE_CLICK_TIME) && (enc_switch_in == false)) {  //Reset Count when no Button Pressed
    enc_switch_counter = 0;
  }

  if ((enc_switch_in == false) && (enc_switch_counter == 0)) {
    enc_switch = 0;
  }

  else if ((enc_switch_in == true) && (enc_switch_counter == 1)) {
    enc_switch = 1;

  }

  else if ((enc_switch_in == true) && (enc_switch_counter >= 2)) {
    enc_switch = 2;
  }


  if (enc_switch  == 0) {//GLOBAL BPM COARSE - INCREASE/DECREASE BPM BY 5 BEATS
    newPosition1 = myEnc.read();
    if ( (newPosition1 - 3) / 4  > oldPosition1 / 4) {
      oldPosition1 = newPosition1;
      oldPosition2 = newPosition1;
      oldPosition3 = newPosition1;
      BPM = BPM - 5;
    }

    else if ( (newPosition1 + 3) / 4  < oldPosition1 / 4 ) {
      oldPosition1 = newPosition1;
      oldPosition2 = newPosition1;
      oldPosition3 = newPosition1;
      BPM = BPM + 5;
    }

    if ( BPM < 60) {
      BPM = 160;
    }
    else if ( BPM >= 161 ) {
      BPM = 60;
    }
  }

  else if (enc_switch  == 1) {//GLOBAL BPM FINE - INCREASE/DECREASE BPM BY 1 BEAT

    newPosition2 = myEnc.read();
    if ( (newPosition2 - 3) / 4  > oldPosition2 / 4) {
      oldPosition1 = newPosition2;
      oldPosition2 = newPosition2;
      oldPosition3 = newPosition2;
      BPM = BPM - 1;
    }

    else if ( (newPosition2 + 3) / 4  < oldPosition2 / 4 ) {
      oldPosition1 = newPosition2;
      oldPosition2 = newPosition2;
      oldPosition3 = newPosition2;
      BPM = BPM + 1;
    }

    if ( BPM < 60) {
      BPM = 160;
    }
    else if ( BPM >= 161 ) {
      BPM = 60;
    }
  }

  else if (enc_switch  == 2) {//DIVISION PROGRAM
    counter = 0;
    newPosition3 = myEnc.read();
    if ( (newPosition3 - 3) / 4  > oldPosition3 / 4) {
      oldPosition1 = newPosition3;
      oldPosition2 = newPosition3;
      oldPosition3 = newPosition3;
      prog = prog - 1;
    }

    else if ( (newPosition3 + 3) / 4  < oldPosition3 / 4 ) {
      oldPosition1 = newPosition3;
      oldPosition2 = newPosition3;
      oldPosition3 = newPosition3;
      prog = prog + 1;
    }

    if ( prog < 0) {
      prog = 23;
    }
    else if ( prog >= 24 ) {
      prog = 0;
    }
  }

  modeSwitch = !digitalRead(14);

  if (modeSwitch == LOW) { //MANUAL PULSE MODE
    mode = 0; 
  }

  else if (modeSwitch == HIGH) { //AUTO MODE
    mode = 1;
  }

}


void segDisplay() {

  if (enc_switch == 0) {
    if (BPM <= 99) {
      tm.display(BPM, false, false, 2); //BLANK FIRST 2 DIGITS
    }

    else if (BPM <= 160) {
      tm.display(BPM, false, false, 1); //BLANK FIRST DIGIT
    }
  }

  else if (enc_switch == 1) {
    if (BPM <= 99) {
      tm.display(BPM, false, false, 2); //BLANK FIRST 2 DIGITS
    }

    else if (BPM <= 160) {
      tm.display(BPM, false, false, 1); //BLANK FIRST DIGIT
    }
  }

  else if (enc_switch == 2) {

    switch (prog) { //DISPLAY TIMING PROGRAM
      case 0:
        tm.display("1,2,4,8,16,32,64,128")->scrollLeft(150);
        break;

      case 1:
        tm.display("1,3,5,7,9,11,13,15")->scrollLeft(150);
        break;

      case 2:
        tm.display("1,2,3,4,5,6,7,8")->scrollLeft(150);
        break;

      case 3:
        tm.display("1,3,5,8,13,21,34,55")->scrollLeft(150);
        break;

      case 4:
        tm.display("1,2,3,5,7,11,13,17")->scrollLeft(150);
        break;

      case 5:
        tm.display("1,3,6,10,15,21,28,36")->scrollLeft(150);
        break;

      case 6:
        tm.display("1,4,9,16,25,36,49,64")->scrollLeft(150);
        break;

      case 7:
        tm.display("1,4,10,20,35,56,84,120")->scrollLeft(150);
        break;

      case 8:
        tm.display("1,5,14,30,55,91,140,204")->scrollLeft(150);
        break;

      case 9:
        tm.display("1,8,27,64,125,216,343,512")->scrollLeft(150);
        break;

      case 10:
        tm.display("1,32,243,1024,3125,7776,16807,32768")->scrollLeft(150);
        break;

      case 11:
        tm.display("1,13,37,73,121,181,253,337")->scrollLeft(150);
        break;

      case 12:
        tm.display("1,14,51,124,245,426,679,1016")->scrollLeft(150);
        break;

      case 13:
        tm.display("1,2,4,8,12,24,48,72")->scrollLeft(150);
        break;

      case 14:
        tm.display("1,16,22,34,36,46,56,64")->scrollLeft(150);
        break;

      case 15:
        tm.display("1,72,108,200,288,392,432,500")->scrollLeft(150);
        break;

      case 16:
        tm.display("1,6,21,28,301,325,496,697")->scrollLeft(150);
        break;

      case 17:
        tm.display("1,2,8,20,28,50,82,126")->scrollLeft(150);
        break;

      case 18:
        tm.display("1,21,33,57,69,77,93,129")->scrollLeft(150);
        break;

      case 19:
        tm.display("1,2,3,2,5,6,7,2")->scrollLeft(150);
        break;

      case 20:
        tm.display("1,30,42,66,70,78,102,105")->scrollLeft(150);
        break;

      case 21:
        tm.display("1,9,45,55,99,297,703,999")->scrollLeft(150);
        break;

      case 22:
        tm.display("1,70,836,4030,5830,7192,7912,9272")->scrollLeft(150);
        break;

      case 23:
        tm.display("1,15,34,65,111,175,260,369")->scrollLeft(150);
        break;
        
      default:
        tm.display("1,2,4,8,16,32,64,128")->scrollLeft(150);
        break;

    }
  }
}

void clockOut() {

  bpmDelay = ((msPerMin / BPM) / 8);
  //    /2 = QUARTER NOTE BEAT
  //    /4 = EIGHTH NOTE BEAT
  //    /8 = SIXTEENTH NOTE BEAT

  currentTime = millis();

  if (mode == LOW) { //MANUAL PULSE MODE
    counter = 0; //RESET COUNTER

    pulseSwitchIn = !digitalRead(15);

    if (pulseSwitchIn == HIGH) {
      digitalWrite(clockLed1, LOW); //SET CLOCK LED LOW IF SWITCH IS HIGH
    }

    else if (pulseSwitchIn == LOW) { //SET CLOCK LED HIGH IF SWITCH IS LOW
      digitalWrite(clockLed1, HIGH);
    }
  }

  else if (mode == 1) { //AUTO MODE
    if (currentTime - previousTime >= bpmDelay) { //MASTER BPM

      counter++; //ADVANCE COUNTER

      // save the last time you blinked the LED
      previousTime = currentTime;

      // if the LED is off turn it on and vice-versa:
      clockState1 = !clockState1;
      // set the LED with the clockState1 of the variable:
      digitalWrite(clockLed1, clockState1);


      if (counter % pgm[prog][0] == 0) { //BPM DIVIDED BY PGM SEQUENCE AT INDEX 0
        // if the LED is off turn it on and vice-versa:
        clockState2 = !clockState2;
        // set the LED with the clockState2 of the variable:
        digitalWrite(clockLed2, clockState2);
      }

      if (counter % pgm[prog][1] == 0) { //BPM DIVIDED BY PGM SEQUENCE AT INDEX 1
        // if the LED is off turn it on and vice-versa:
        clockState3 = !clockState3;
        // set the LED with the clockState3 of the variable:
        digitalWrite(clockLed3, clockState3);
      }

      if (counter % pgm[prog][2] == 0) { //BPM DIVIDED BY PGM SEQUENCE AT INDEX 2
        // if the LED is off turn it on and vice-versa:
        clockState4 = !clockState4;
        // set the LED with the clockState4 of the variable:
        digitalWrite(clockLed4, clockState4);
      }

      if (counter % pgm[prog][3] == 0) { //BPM DIVIDED BY PGM SEQUENCE AT INDEX 3
        // if the LED is off turn it on and vice-versa:
        clockState5 = !clockState5;
        // set the LED with the clockState5 of the variable:
        digitalWrite(clockLed5, clockState5);
      }

      if (counter % pgm[prog][4] == 0) { //BPM DIVIDED BY PGM SEQUENCE AT INDEX 4
        // if the LED is off turn it on and vice-versa:
        clockState6 = !clockState6;
        // set the LED with the clockState6 of the variable:
        digitalWrite(clockLed6, clockState6);
      }

      if (counter % pgm[prog][5] == 0) { //BPM DIVIDED BY PGM SEQUENCE AT INDEX 5
        // if the LED is off turn it on and vice-versa:
        clockState7 = !clockState7;
        // set the LED with the clockState7 of the variable:
        digitalWrite(clockLed7, clockState7);
      }

      if (counter % pgm[prog][6] == 0) { //BPM DIVIDED BY PGM SEQUENCE AT INDEX 6
        // if the LED is off turn it on and vice-versa:
        clockState8 = !clockState8;
        // set the LED with the clockState8 of the variable:
        digitalWrite(clockLed8, clockState8);
      }
    }
  }
}

void resetcntr() {

  if (counter == 0) {
    
    currentTime = millis();
    
    clockState2 = LOW;
    clockState3 = LOW;
    clockState4 = LOW;
    clockState5 = LOW;
    clockState6 = LOW;
    clockState7 = LOW;
    clockState8 = LOW;

    digitalWrite(clockLed2, clockState2);
    digitalWrite(clockLed3, clockState3);
    digitalWrite(clockLed4, clockState4);
    digitalWrite(clockLed5, clockState5);
    digitalWrite(clockLed6, clockState6);
    digitalWrite(clockLed7, clockState7);
    digitalWrite(clockLed8, clockState8);
  }
}
