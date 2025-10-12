/*
 * Arduino Uno Clock, Alarm, Timer, Stopwatch Hybrid Device
 * by Hemachander Rubeshkumar, Haris Iqbal, and Danh Bui
 * 
 * Using the Following Library:
 * Library Name: Grove 4Digital Display
 * Author: Frankie.Chu for Seeed Technology Inc 
 * Version: 1.0.0.
 * Source: https://github.com/Seeed-Studio/Grove_4Digital_Display 
 * License Type: MIT 
 */

//Display
#include <TM1637.h>
int clk = 12;
int dio = 13;
TM1637 displayScreen(clk,dio);

int clkTime[2] = {0, 0}; 
int stpTime[2] = {0, 0};
int tmrTime[2] = {0, 0};
int alarmTime[2] = {0, 0};
int displayDigits[4] = {0,0,0,0};
//buttons
int moveButton = 2;
int increaseButton = 3;
int setButton = 4;
int modeButton = 5;
//seconds
int seconds = 0;
int centisec = 0;
//mode
int mode = 0;
bool timerActive = false;
bool alarmActive = false;
bool stopActive = false;
//leds
int led1 = 7;
int led2 = 8;
int led3 = 9;
//buzzer
int buzzer = 6;

void setup() {
  //pinMode
  pinMode(moveButton, INPUT);
  pinMode(increaseButton, INPUT);
  pinMode(setButton, INPUT);
  pinMode(modeButton, INPUT);
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);
  pinMode(buzzer, OUTPUT);
  //initialize display
  displayScreen.init();
  //brightness (0-7)
  displayScreen.set(3);
  //Booting up
  for (int j = 0; j < 6; j++){
    for (int i = 0; i < 6; i++){
    displayScreen.display(i,8*(j%2));
    }
    delay(500);
  }
  
  //user sets clock time
  setClockTime();
}

void setClockTime(){
  int activeDigitIndex = 0;
  while(!buttonPress(setButton)){
    if (buttonPress(increaseButton)){
      displayDigits[activeDigitIndex] = (displayDigits[activeDigitIndex]+1)%10;
    }
    if (buttonPress(moveButton)){
      activeDigitIndex = (activeDigitIndex+1)%4;
    }
    if (buttonPress(modeButton)){
      mode = (mode + 1)%5;
      return true;
    }
    for(int i = 0; i < 4; i++){
      displayScreen.display(i, displayDigits[i]);
    }
  }
  //Sets the displayed time into the clock time
      //sets hours
      clkTime[0] = displayDigits[0]*10+displayDigits[1];
      //sets minutes
      clkTime[1] = displayDigits[2]*10+displayDigits[3];
  for(int i = 0; i < 4; i++){
      displayDigits[i] = 0;
  }
  mode = (mode+1)%5;
}

void setAlarmTime(){
  int activeDigitIndex = 0;
  while(!buttonPress(setButton)){
    if (buttonPress(increaseButton)){
      displayDigits[activeDigitIndex] = (displayDigits[activeDigitIndex]+1)%10;
    }
    if (buttonPress(moveButton)){
      activeDigitIndex = (activeDigitIndex+1)%4;
    }
    if (buttonPress(modeButton)){
      alarmTime[0] = 0;
      alarmTime[1] = 0;
      alarmActive = false;
      mode = (mode + 1)%5;
      return true;
    }
    for(int i = 0; i < 4; i++){
      displayScreen.display(i, displayDigits[i]);
    }
  }
  //Sets displayed time into the alarm time that gets checked by update time
  //sets hours
  alarmTime[0] = displayDigits[0]*10+displayDigits[1];
  //sets minutes
  alarmTime[1] = displayDigits[2]*10+displayDigits[3];
  for(int i = 0; i < 4; i++){
      displayDigits[i] = 0;
  }
  mode = (mode+1)%5;
  alarmActive = true;
}

void setTimerTime(){
  int activeDigitIndex = 0;
  for (int i = 0; i < 4; i++){
    displayDigits[i] = 0;
    }
  while(!buttonPress(setButton)){
    updateDisplay(displayDigits[0]*1000 + displayDigits[1]*100 + displayDigits[2]*10 + displayDigits[3]);
    if (buttonPress(increaseButton)){
      displayDigits[activeDigitIndex] = (displayDigits[activeDigitIndex]+1)%10;
      }
    if (buttonPress(moveButton)){
      activeDigitIndex = (activeDigitIndex+1)%4;
      }
    if (buttonPress(modeButton)){
      tmrTime[0] = 0;
      tmrTime[1] = 0;
      //move to stopwatch
      mode = 2;
      timerActive = false;
      return;
    }
  }
  timerActive = true;
  //sets seconds
  tmrTime[0] = displayDigits[0]*10+displayDigits[1];
  //sets centisec
  tmrTime[1] = displayDigits[2]*10+displayDigits[3];
}

void updateStopwatch(){
    stpTime[1]+=1;
    if (stpTime[1]/100 > 0){
      stpTime[0] += stpTime[1]/100;
      seconds += stpTime[1]/100;
      stpTime[1] = stpTime[1]%100;
      }
    if (stpTime[0] > 99){
      stpTime[0] = 99;
      stpTime[1] = 99;
      ringAlarm();
      }
  }

void updateTimer(){
  if(tmrTime[1]<=0){
    if(tmrTime[0]<=0){
      //At 00:00 or lower
      ringAlarm();
      timerActive = false;
      }
    else {
      //At XX:00 -> XX-1:99
      tmrTime[0]-=1;
      tmrTime[1]+=100;
    }
  }
  else{
    tmrTime[1]-=1;
  }
}

//updates clock, timer, and stopwatch
// Works for Clock and Timer
void updateTime(){
  centisec++;
  //updates in centiseconds because of the stopwatch and timer
  if(centisec/100 > 1){
    seconds += centisec/100;
    centisec = centisec%100;
    }
  //Carries over the centiseconds into seconds
  if (seconds > 59){
    clkTime[1] += seconds/60;
    seconds = seconds%60;
  }
  //Carries the seconds into minutes 
  if (clkTime[1] > 59){
    clkTime[0] += clkTime[1]/60;
    clkTime[1] = clkTime[1]%60;
  }
  //Carries the minutes into hours
  if (clkTime[0] > 24){
    //military time, so 1-24
    clkTime[0] = (clkTime[0]%24) + 1;
    }
  //Bugged
  //Check for alarm (if active and time is equal then ring alarm)
  if (alarmActive && ((clkTime[1]==alarmTime[1]) && (clkTime[0]==alarmTime[0]))){
    ringAlarm();
    alarmActive = false;
  }
  //Update Timer
  if (timerActive){
    updateTimer();
  }
  //Update Stopwatch
  if (stopActive){
    updateStopwatch();
    }
  delay(10);
}

//updates the display with the input
void updateDisplay(int counter){
   for (int i = 0; i < 4; i++){
     //display(position, character)
     displayScreen.display(3-i, counter%10);
     counter/=10;
   }
}

//delay function would mess with our time keeping, so this function helps us get around that
void wait(int ms){
  delay(ms);
  centisec += ms/10;
  }

void ringAlarm(){
  //Rings Alarm for 15 seconds
  tone(buzzer, 1000, 15000);
}

bool buttonPress(int buttonPin){
  if (digitalRead(buttonPin)==1){
    delay(150);
    return true;
  } else {
    return false;
  }
}

void loop(){
  //Covers time updates
  updateTime();
  if(buttonPress(modeButton)){
    mode = (mode+1)%5;
  }
  //default output (treat output as an array of 4 integers)
  //this section should update the display
  switch(mode){
    case 0:
      //clock mode, no change
      updateDisplay(clkTime[0]*100 + clkTime[1]);
      //leds
      digitalWrite(led1, HIGH);
      digitalWrite(led2, LOW);
      digitalWrite(led3, LOW);
      break;
    case 1:
      //timer mode
      digitalWrite(led1, LOW);
      digitalWrite(led2, LOW);
      digitalWrite(led3, HIGH);
      if(!timerActive){
        //Set up the timer
        setTimerTime();
      }
      else{
        //Stop or Cancel the timer
        if (buttonPress(setButton)||buttonPress(moveButton)||buttonPress(increaseButton)){
          timerActive = false;
          }
        updateDisplay(tmrTime[0]*100 + tmrTime[1]);
      }
      break;
    case 2:
      //stopwatch mode
      digitalWrite(led1, LOW);
      digitalWrite(led2, HIGH);
      digitalWrite(led3, LOW);
      if(!stopActive){
        //start stopwatch
        if(buttonPress(setButton)){
          stpTime[0] = 0;
          stpTime[1] = 0;
          stopActive = true;
        } 
      }
      //buttons to stop the stopwatch after it has started
      else if (buttonPress(setButton)||buttonPress(moveButton)||buttonPress(increaseButton)||buttonPress(modeButton)){
        stopActive = false;
      }
      updateDisplay(stpTime[0]*100 + stpTime[1]);
      break;
    case 3:
      //alarm mode
      digitalWrite(led1, HIGH);
      digitalWrite(led2, LOW);
      digitalWrite(led3, HIGH);
      setAlarmTime();
      break;
    case 4:
      //set clock time
      digitalWrite(led1, HIGH);
      digitalWrite(led2, HIGH);
      digitalWrite(led3, LOW);
      setClockTime();
      break;
    //contact manufacturer
    default:
      digitalWrite(led1, HIGH);
      digitalWrite(led2, HIGH);
      digitalWrite(led3, HIGH);
  }
}
