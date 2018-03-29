#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Bounce2.h>
#include <OneWire.h>
#include <EEPROM2.h>

#define KEY_UP 4
#define KEY_DN 5
#define KEY_OK 3
#define DS_PIN 2
#define R1_PIN 13

Adafruit_SSD1306 oled;
OneWire ds(DS_PIN);
Bounce keyUP = Bounce();
Bounce keyDN = Bounce();
Bounce keyOK = Bounce();

byte water_sens[8] = {0x28,0xFF,0x53,0x81,0xA4,0x15,0x03,0x5C};
byte engine_sens[8] = {0x28,0xFF,0x8F,0xDA,0xA4,0x15,0x03,0x6F};

float fan_on_temp = 90.0;
float fan_off_temp = 83.0;
float current_water_temp = 0;
float engine_temp = 0;
boolean disp_mode = 0;
boolean relay_mode = 0;
uint8_t mode = 0;
uint8_t sp = 2;

unsigned long SENS_prevMillis = 0; 
const long SENS_interval = 1000;
unsigned long CHANGE_prevMillis = 0; 
const long CHANGE_interval = 500;

float GetTemp(byte *sensor){
  byte data[12];
  ds.reset();
  ds.select(sensor);
  ds.write(0x44, 1);
  delay(50); 
  ds.reset();
  ds.select(sensor);
  ds.write(0xBE);

  for (byte i = 0; i < 10; i++){ 
    data[i] = ds.read();
  }
  int tmp=(data[1] << 8) | data[0];
  float temp =  (float)tmp / 16.0;
  return temp;
}

void disp(boolean state){
  if(!state){    
    oled.setTextSize(1);
    oled.setTextColor(WHITE);
    oled.setCursor(0,0);
    oled.println("WATER TEMP");  
    oled.setTextSize(3);
    oled.setTextColor(WHITE);
    oled.print(current_water_temp,1);oled.print((char)247); oled.print("C");
    oled.display();
    //delay(2000);
    oled.clearDisplay();    
  }
  else {
    oled.setTextSize(1);
    oled.setTextColor(WHITE);
    oled.setCursor(0,0);
    oled.println("ENGINE TEMP");  
    oled.setTextSize(3);
    oled.setTextColor(WHITE);
    oled.print(engine_temp,1);oled.print((char)247); oled.print("C");
    oled.display();
    //delay(2000);
    oled.clearDisplay();    
  }  
}

void show(){
  if(mode==0) disp(disp_mode);
  if(mode==1) ChangeOnTemp();
  if(mode==2) ChangeOffTemp();
  if(mode==3) saveParam(); 
}

void ChangeOnTemp(){
  oled.setTextSize(1);
  oled.setTextColor(WHITE);
  oled.setCursor(0,0);
  oled.println("FAN ON TEMP");  
  oled.setTextSize(3);
  oled.setTextColor(WHITE);
  oled.print(fan_on_temp,1);oled.print((char)247); oled.print("C");
  oled.display();
  //delay(2000);
  oled.clearDisplay();   
}

void ChangeOffTemp(){
  oled.setTextSize(1);
  oled.setTextColor(WHITE);
  oled.setCursor(0,0);
  oled.println("FAN OFF TEMP");  
  oled.setTextSize(3);
  oled.setTextColor(WHITE);
  oled.print(fan_off_temp,1);oled.print((char)247); oled.print("C");
  oled.display();
  //delay(2000);
  oled.clearDisplay();
}

void saveParam(){
  oled.setTextSize(1);
  oled.setTextColor(WHITE);
  oled.setCursor(0,0);
  oled.println("SAVE PARAM?");
  oled.setTextSize(3);
  oled.setTextColor(WHITE);
  oled.print("YES-UP");
  oled.display();
  //delay(2000);
  oled.clearDisplay();
  switch(sp){
    case 0:
    mode=0;
    sp=2;
    break;
    case 1:
    EEPROM_write(8,fan_on_temp);
    delay(5);
    EEPROM_write(16,fan_off_temp);
    delay(5);
    oled.setTextSize(2);
    oled.setTextColor(WHITE);
    oled.setCursor(0,0);
    oled.println("PARAM");
    oled.setTextSize(2);
    oled.setTextColor(WHITE);
    oled.print("SAVED");
    oled.display();
    delay(2000);
    oled.clearDisplay();
    mode=0;
    sp=2;
    break;
    case 2:
    mode=3;
    sp=2;
    break;
  }
}


void setup(){
  oled.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  keyUP.attach(KEY_UP); 
  keyUP.interval(5);
  keyDN.attach(KEY_DN); 
  keyDN.interval(5);
  keyOK.attach(KEY_OK); 
  keyOK.interval(5);
 
  pinMode(R1_PIN, OUTPUT);
  digitalWrite(R1_PIN, HIGH);
  pinMode(KEY_UP, INPUT_PULLUP);
  pinMode(KEY_DN, INPUT_PULLUP);
  pinMode(KEY_OK, INPUT_PULLUP); 
  pinMode(12, INPUT); 

  delay(5);
  EEPROM_read_mem(8, &fan_on_temp, sizeof(fan_on_temp));
  delay(5);
  EEPROM_read_mem(16, &fan_off_temp, sizeof(fan_off_temp));

  if(digitalRead(12) == 0) relay_mode = 0;
  else relay_mode = 1;
  
}

 
void loop() {
  unsigned long currentMillis = millis();

  keyUP.update();
  keyDN.update();
  keyOK.update();

  if (currentMillis - SENS_prevMillis >= SENS_interval){
    SENS_prevMillis = currentMillis;    
    current_water_temp=GetTemp(water_sens);
    delay(10);
    engine_temp=GetTemp(engine_sens);    
  }  
 

  if(keyOK.fell()) mode++;

  if(mode==0){
    if(keyUP.fell()) disp_mode=1;
    if(keyDN.fell()) disp_mode=0;
  }
  if(mode==1){
    if(keyUP.read() == 0 && currentMillis - CHANGE_prevMillis >= CHANGE_interval) {
      CHANGE_prevMillis = currentMillis;
      fan_on_temp+=0.5;
    }
    if(keyDN.read() == 0 && currentMillis - CHANGE_prevMillis >= CHANGE_interval) {
      CHANGE_prevMillis = currentMillis;
      fan_on_temp-=0.5;
    }
  }
  if(mode==2){
    if(keyUP.read() == 0 && currentMillis - CHANGE_prevMillis >= CHANGE_interval) {
      CHANGE_prevMillis = currentMillis;
      fan_off_temp+=0.5;
    }
    if(keyDN.read() == 0 && currentMillis - CHANGE_prevMillis >= CHANGE_interval) {
      CHANGE_prevMillis = currentMillis;
      fan_off_temp-=0.5;
    }
  }
  if(mode==3){
    if(keyUP.fell()) sp=1;
    if(keyDN.fell()) sp=0;
  }
  if(mode>3) mode = 0;

  show();

  if(relay_mode){
    if(current_water_temp >= fan_on_temp) digitalWrite(R1_PIN, LOW);
    if(current_water_temp <= fan_off_temp) digitalWrite(R1_PIN, HIGH);
  }

  if(!relay_mode){
    if(current_water_temp >= fan_on_temp) digitalWrite(R1_PIN, HIGH);
    if(current_water_temp <= fan_off_temp) digitalWrite(R1_PIN, LOW);
  }

  //delay(50);
}
