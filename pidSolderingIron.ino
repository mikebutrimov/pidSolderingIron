#include <PID_v1.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
// pin 9 - Serial clock out (SCLK)
// pin 8 - Serial data out (DIN)
// pin 7 - Data/Command select (D/C)
// pin 6 - LCD chip select (CS)
// pin 5 - LCD reset (RST)
Adafruit_PCD8544 display = Adafruit_PCD8544(6, 5, 4, 3, 2);
int oldTemperature = 220;
int newTemperature = 220;
int defaultTemperature = 220;
int maxTemperature = 300;
int pwnOutputPin = 12;
int pwmThrottle = 0;
int temperatureInputPin = 17;
int currentTime = 0; 
int lastTimeCall = 0;
int secondToLastTimeCall;
int upButtonPin = 19;
int downButtonPin = 20;
unsigned long coolDownOnce = 0; 
unsigned long coolDownRepeat = 0; 
unsigned long coolDownOnStartRepeat = 0;
unsigned long coolDownDisplayRepeat = 0;
int coolDownOnceTrigger = 0;
int coolDownRepeatTrigger = 0;
int coolDownOnStartRepeatTrigger = 0;
int coolDownDisplayTrigger = 0;
int upButtonState = 0;
int upButtonIsPressed = 0;
int upButtonWasPressed= 0;
 
int downButtonState = 0;
int downButtonIsPressed = 0;
int downButtonWasPressed = 0;

int contDelay = 100;
int onceDelay = 20;
int onStartRepeatDelay = 500;
int displayDelay = 100;

int displayTempType = 0; // 0 - current temp, 1 - new target temp
int needClearDisplay = 0;

/////PID Section

double Input,Output,Setpoint;
PID termalPID(&Input, &Output, &Setpoint,1,4,3, DIRECT);
///////////////



void displayMsg(String msg, Adafruit_PCD8544 display, int line = 0, int tab = 0, int txtSize = 1){
  display.display();
  display.setCursor(line, tab);
  display.setTextColor(BLACK);
  display.setTextSize(txtSize);
  int len = msg.length()+1;
  char str[len];
  msg.toCharArray(str,len);
  for (int i = 0; i<len; i++){
    display.write(str[i]);
  } 
}
 
void processButtons(){
  upButtonState = digitalRead(upButtonPin);
  downButtonState = digitalRead(downButtonPin);
  if (upButtonState == 0 && upButtonIsPressed == 0){
    //do fucking nothing
  }
  if (downButtonState == 0 && downButtonIsPressed == 0){
    //do fucking nothing
  }
  if (upButtonState == 1 && upButtonIsPressed == 0){
    //rise fucking temperature
    newTemperature = riseTemperature(oldTemperature);
    upButtonIsPressed = 1;
    //
    displayTempType=1;
  }
  if (downButtonState == 1 && downButtonIsPressed == 0){
    //lower fucking temperature
    newTemperature = lowerTemperature(oldTemperature);
    downButtonIsPressed = 1;
    //
    displayTempType=1;
  }
  if (upButtonState == 1 && upButtonIsPressed == 1){
    if (coolDownOnStartRepeatTrigger == 0){
      coolDownOnStartRepeat = millis() + onStartRepeatDelay;
      coolDownOnStartRepeatTrigger = 1;
    }
    if (millis() > coolDownOnStartRepeat){      
      //rise fucking temperature
      if (coolDownRepeatTrigger == 0){
      coolDownRepeat = millis()+contDelay;
      coolDownRepeatTrigger = 1;
    }
    if (millis() > coolDownRepeat){  
      newTemperature = riseTemperature(oldTemperature);
      coolDownRepeatTrigger = 0;
    }
    }
  upButtonIsPressed = 1;
  }
  if (downButtonState == 1 && downButtonIsPressed == 1){
    //lower fucking temperature
      if (coolDownOnStartRepeatTrigger == 0){
        coolDownOnStartRepeat = millis() + onStartRepeatDelay;
        coolDownOnStartRepeatTrigger = 1;
      }
    if (millis() > coolDownOnStartRepeat){      
      if (coolDownRepeatTrigger == 0){
      coolDownRepeat = millis()+contDelay;
      coolDownRepeatTrigger = 1;
      }
      if (millis() > coolDownRepeat){  
        newTemperature = lowerTemperature(oldTemperature);
        coolDownRepeatTrigger = 0;
      }
    }
    downButtonIsPressed = 1;
  }
  if (upButtonState == 0 && upButtonIsPressed == 1){
    //release fucking button
    upButtonIsPressed = 0;
    coolDownRepeatTrigger = 0;
    coolDownOnStartRepeatTrigger = 0;
    displayTempType=0;
    needClearDisplay = 1;
  }
  if (downButtonState == 0 && downButtonIsPressed == 1){
    //release fucking button
    downButtonIsPressed = 0;
    coolDownRepeatTrigger = 0;
    coolDownOnStartRepeatTrigger = 0;
    displayTempType=0;
    needClearDisplay = 1;
  }
}
 
int riseTemperature (int temp){
  int returnedTemp = temp+5;
  if (returnedTemp <= maxTemperature){
    return returnedTemp;
  }
  else {
    return temp;
  }
}
 
int lowerTemperature (int temp){
  int returnedTemp = temp-5;
  if (returnedTemp < 0){
    return temp;
  }
  return returnedTemp;
}

void setup() {
  Serial.begin(115200);
  pinMode(pwnOutputPin, OUTPUT);
  pinMode(temperatureInputPin, INPUT);
  pinMode(upButtonPin, INPUT);
  pinMode(downButtonPin, INPUT);
  analogWrite(pwnOutputPin,pwmThrottle);
  display.begin();
  display.clearDisplay();
  display.display();
  display.setContrast(60);
  delay(1000);
}

void loop() {
  //here we read out temperature settings and generate fuzzy objects
  //now just sustitle it with randomly generated number
  if (coolDownOnceTrigger ==0){
    coolDownOnce = millis()+onceDelay;
    coolDownOnceTrigger = 1;
  }
  if (millis() > coolDownOnce){  
    processButtons();
    coolDownOnceTrigger = 0;
  }
  if (newTemperature != oldTemperature){
    oldTemperature = newTemperature;
    Serial.print("Create Fuzzy");
    Serial.println();
  }
  int currentTemp = ((analogRead(temperatureInputPin)*0.6)+20);
  

  if (coolDownDisplayTrigger ==0){
    coolDownDisplayRepeat = millis()+displayDelay;
    coolDownDisplayTrigger = 1;
  }
  if (millis() > coolDownDisplayRepeat){  
    if (displayTempType==1){
      display.clearDisplay();
      displayMsg("New target:", display);
      displayMsg((String)newTemperature,display, 0, 9, 4);
      display.display();
        for (int y = 9; y< 38; y++){
          for (int x = 0; x< 84; x++){
            display.drawPixel(x,y,0);
          }
        }
    } 
    
    if (displayTempType == 0){  
      if (needClearDisplay == 1){
        display.display();
        display.clearDisplay();
        needClearDisplay = 0;        
      }
      displayMsg("Current temp:", display);
      displayMsg((String)currentTemp,display, 0, 9, 4);
      displayMsg("pwm: ",display,0,38,1);
      displayMsg((String)pwmThrottle,display,25,38,1);
      display.display();
      for (int y = 9; y< 37; y++){
        for (int x = 0; x< 84; x++){
          display.drawPixel(x,y,0);
        }
      }
      for (int y = 38; y< 48; y++){
        for (int x = 24; x< 84; x++){
          display.drawPixel(x,y,0);
        }
      }
    }
    coolDownDisplayTrigger = 0;
  }
  

  Serial.print("New temperature -> ");
  Serial.print(newTemperature);
  Serial.println();
  Serial.print("Action - > ");
  Serial.print(pwmThrottle);
  Serial.println();
  Input = currentTemp;
  Setpoint = newTemperature;
  termalPID.Compute();
  analogWrite(pwnOutputPin,Output);
  Serial.print("Thermal Readout? -> ");
  Serial.print(currentTemp);
  Serial.println();
 
  delay(100);

}
