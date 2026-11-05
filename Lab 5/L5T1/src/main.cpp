#include <Arduino.h>
const int led = 23;     
const int pwmChannel = 0;   
const int freq = 5000;      
const int resolution = 8; 
  int stepsize = 26;
  int value = 0;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  ledcSetup(pwmChannel, freq, resolution);
  ledcAttachPin(led, pwmChannel);
}



void loop() {
  value += stepsize;
  if(value>255){
    value = 0;
    ledcWrite(pwmChannel, value);
  }
  else{
    Serial.println(value);
    ledcWrite(pwmChannel, value);
  } 
  delay(2000);

}