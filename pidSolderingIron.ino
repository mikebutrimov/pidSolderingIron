#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include <PID_v1.h>
Adafruit_PCD8544 display = Adafruit_PCD8544(6, 5, 4, 3, 2);

class Button {
  public:
    Button(int pin);
    int getState();
    int processState();
  private:
    int _pin;
    int _shortPressDelay;
    int _longPressDelay;
    int _buttonState; //0 - released; 1 - short press; 2 - in long press; 
                      //3 - was in short press and now waits for long or release
    unsigned long  _shortPressTimer;
    unsigned long  _longPressTimer;
};

Button upButton = Button(11);
Button downButton = Button(12);
int newTemperature = 220;
int oldTemperature = 220;
int maxTemperature = 300;
int currentTemp = 0;
int oldCurrentTemp = 0;
int pwnOutputPin = 10;
int temperatureInputPin = 17;
float pwmThrottle = 0;
int clearDisplay = 0;  
unsigned long coolDownDisplay = 0;
int displayDelay = 2000;
double Input,Output,Setpoint;
PID termalPID(&Input, &Output, &Setpoint,1,4,3, DIRECT);

Button::Button(int pin){
  //LES CONSTRUTOR
  _pin = pin;
  _shortPressDelay = 5;
  _longPressDelay = 400;
  _buttonState = 0;
  _shortPressTimer = 0;
  _longPressTimer = 0; 
  pinMode(pin,INPUT);
}
int Button::getState(){
  int readout0 = digitalRead(_pin);
  delay(1);
  int readout1 = digitalRead(_pin);
  if (readout0 == readout1){
    return readout1;
  }
  else {
    return 0;
  }
}

int Button::processState(){
  int _state = getState();
  //check timer states
  if (_shortPressTimer == 0 && _longPressTimer == 0){
    //timers both are off, we start from full unpressed mode
    if (_buttonState == 0 && _state == 1){
      //button was not pressed and now reads as pressed
      //fire up both timers
      _shortPressTimer = millis();
      _longPressTimer = millis();
      _buttonState = 1;
    }
  }
  if (_shortPressTimer != 0 and _longPressTimer !=0 ){
    unsigned long currentTime = millis();
    if (currentTime > (_longPressTimer + _longPressDelay)){
      if (_buttonState == 3 && _state == 1){
        _buttonState = 2;
        
      }
      if (_buttonState > 0 && _state == 0){
        _longPressTimer = 0;
        _shortPressTimer = 0;
        _buttonState = 0;
      }
    }
  else if (currentTime > (_shortPressTimer + _shortPressDelay)){
      if (_buttonState > 0 && _state == 0){
        _longPressTimer = 0;
        _shortPressTimer = 0;
        _buttonState = 0;
      }
      else if (_buttonState == 1 && _state == 1 && 
                currentTime < (_longPressTimer + _longPressDelay)){
        _buttonState = 3;
      }
    } 
  }
if (_buttonState == 3){
  return 0;
}
return _buttonState;
}

void processButtons(){
  int upButtonState = upButton.processState();
  int downButtonState = downButton.processState();
  if (upButtonState == downButtonState){
    //do fucking nothing!
  }
  if (upButtonState > 0){
    if (upButtonState == 2){
      delay(200);  
      newTemperature = riseTemperature(oldTemperature);
    }
    else {
      newTemperature = riseTemperature(oldTemperature);
    }    
    coolDownDisplay = millis();
  }
  if (downButtonState > 0){
    if (downButtonState == 2){
      delay(200);  
      newTemperature = lowerTemperature(oldTemperature);
    }
    else {
      newTemperature = lowerTemperature(oldTemperature);
    }    
    coolDownDisplay = millis();
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

void displayDisplayable(int newTemperature, int oldTemperature){
  if (millis() < (coolDownDisplay + displayDelay)){
    if (newTemperature!=oldTemperature){
      display.clearDisplay();
      for (int y = 9; y< 38; y++){
        for (int x = 0; x< 84; x++){
          display.drawPixel(x,y,0);
        }
      }
    }
    displayMsg("New target:", display);
    displayMsg((String)newTemperature,display, 0, 9, 4);
    display.display();
    clearDisplay = 1;
  }
  else {
    if (clearDisplay == 1){
      display.clearDisplay();
      clearDisplay = 0;
    }
    int ololo;  
    int ct0 = currentTemp / 100;
    int ct1 = (currentTemp % 100) / 10;
    int ct2 = (currentTemp % 10);
    int oct0 = oldCurrentTemp / 100;
    int oct1 = (oldCurrentTemp % 100) / 10;
    int oct2 = (oldCurrentTemp % 10);
    if (ct0 !=0 ){
      ololo = 0;
      if (ct0 == oct0){
        ololo = 24;
      }
      if (ct0 == oct0 && ct1 == oct1){
        ololo = 56;
      }
      if (ct0 == oct0 && ct1 == oct1 && ct2 == oct2){
        ololo = 84;
      }
    }
    if ( ct0 == 0){
      ololo = 0;
      if (ct1 == oct1){
        ololo = 24;
      }
      if (ct1 == oct1 && ct2 == oct2){
        ololo = 56;
      }
      
    }

    displayMsg("Current temp:", display);
    displayMsg((String)currentTemp,display, 0, 9, 4);
    displayMsg("pwm: ",display,0,38,1);
    displayMsg((String)pwmThrottle,display,25,38,1);
    display.display();
        
    for (int y = 9; y< 37; y++){
      for (int x = ololo;x<84;x++){
        display.drawPixel(x,y,0);
      }
    }
    for (int y = 38; y< 48; y++){
      for (int x = 24; x< 84; x++){
        display.drawPixel(x,y,0);
      }
    }
  }
}

void setup() {
  Serial.begin(115200);
  termalPID.SetMode(AUTOMATIC);
  pinMode(pwnOutputPin, OUTPUT);
  pinMode(temperatureInputPin, INPUT);
  display.begin();
  display.clearDisplay();
  display.display();
  display.setContrast(60);
  delay(1000);
}

void loop() {
  processButtons();
  currentTemp = ((analogRead(temperatureInputPin)*0.6)+20);
  Input = currentTemp;
  Setpoint = newTemperature;
  termalPID.Compute();
  analogWrite(pwnOutputPin,Output);
  pwmThrottle = Output;
  displayDisplayable(newTemperature, oldTemperature);
  if (newTemperature != oldTemperature){
    oldTemperature = newTemperature;
  }
  oldCurrentTemp = currentTemp;
  //Serial.println(millis());
  delay(50);
}




