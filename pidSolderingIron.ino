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


Button lesButton = Button(26);
Button upButton = Button(30);
Button downButton = Button(25);
int newTemperature = 0;
int oldTemperature = 0;
int maxTemperature = 300;
int currentTemperature = 0;
unsigned long coolDownDisplay = 0;
int displayDelay = 2000;
double Input,Output,Setpoint;
PID termalPID(&Input, &Output, &Setpoint,1,4,3, DIRECT);

void processButtons(){
  int upButtonState = upButton.processState();
  //int downButtonState = downButton.processState();
  int downButtonState = 0;
  if (upButtonState == downButtonState){
    //do fucking nothing!
  }
  if (upButtonState > 0){
    if (upButtonState == 2){
      if ((millis()/300)%3 == 2){
        newTemperature = riseTemperature(oldTemperature);
      }
    }
    else {
      newTemperature = riseTemperature(oldTemperature);
    }    
    coolDownDisplay = millis();
  }
  if (downButtonState > 0){
    newTemperature = lowerTemperature(oldTemperature);
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

void displayDisplayable(int newTemperature, int oldTemperature){
    
  if (millis() < (coolDownDisplay + displayDelay)){
    Serial.println("outputing target temp");
  }
  else {
    Serial.println("output CURRENT temp and pwm");
  }
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


void setup() {
  Serial.begin(115200);
}

void loop() {
  processButtons();
  displayDisplayable(newTemperature, oldTemperature);

  Serial.print(newTemperature);
  Serial.print("       ");
  Serial.println(oldTemperature);
  oldTemperature = newTemperature;
  delay(100);

}




