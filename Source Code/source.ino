#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>

byte sensorInterrupt = 0;

SoftwareSerial ble(0,1);
LiquidCrystal_I2C lcd(0x27, 16, 2);

const int sensor =2;
int but[3] = { 6, 7, 8}, relay = 4;
float val[3] = {200, 700, 1000};
String noti[3] = {"Pouring 200ml", "Pouring 700ml", "Pouring 1000ml"};
float calibrationFactor = 109.52;
float flowRate;
float flow;
float total;
float percent;
unsigned long oldTime;
volatile byte pulseCount;

void setup() {
  Serial.begin(9600);
  ble.begin(9600);
  lcd.init(); lcd.backlight();
  for (int i = 0; i < 3; i++) {
    pinMode(but[i], INPUT_PULLUP);
  }
  pinMode(sensor, INPUT);
  pinMode(relay, OUTPUT);
  digitalWrite(sensor, HIGH);
  digitalWrite(relay, HIGH);
  attachInterrupt(digitalPinToInterrupt(sensor), increase, FALLING);
  lcd.init();
  lcd.backlight();
  wel();
  pulseCount = 0;
  oldTime = millis();
  flowRate = 0;
  flow = 0;
  percent = 0;
}

void loop() {
   for (int i=0; i<3; i++){
     if (digitalRead(but[i]) == LOW){
       lcd.clear();
       dplay(0, 0, "Exec!");
       dplay(0, 1, noti[i]);
       exe1c(val[i]);
       wel();
       return; // ngan xung dot 
     }
   }

  if (ble.available()){
    int cmd = ble.parseInt();
    if (cmd >= 0 && cmd <= 2) {
      lcd.clear();
      dplay(0, 0, "Exec!");
      dplay(0, 1, noti[cmd]);
      exe1c(val[cmd]);
      wel();
    }
    if (cmd == 3){
      lcd.clear();
      dplay(0, 0, "Custom value!");
      dplay(0, 1, "Inp via terminal");
      while (1==1){
        float bval = ble.parseFloat();
        Serial.println(bval);
        if (bval>=200.0 && bval <= 2000.0){
          lcd.clear();
          dplay(0, 0, "Exec!");
          dplay(0, 1, "Pouring " + String(bval) + "mL");
          exe1c(bval);
          wel();
          return;
        }
      }
    }
  }
  
}

void doThing(float vol){
    if (millis() - oldTime >= 1000){
      detachInterrupt(digitalPinToInterrupt(sensor));
      flowRate = pulseCount / calibrationFactor;
      oldTime = millis();
      flow = flowRate / 60.0 * 1000.0;
      total += flow;
      Serial.print("Flow rate: ");
      Serial.print(int(flow));
      Serial.println("ml/s");
      Serial.print("Output Liquid Quantity: ");
      Serial.print(total);
      Serial.println("mL");
      pulseCount = 0;
      percent = (total / vol)*100;
      if (percent>=100){
        percent = 100;
        digitalWrite(relay, HIGH);
      }
      lcd.clear();
      dplay(0, 0, String(percent)+"%");
      attachInterrupt(digitalPinToInterrupt(sensor), increase, FALLING);
    }
}

void exe1c(float vol) {
  total = 0;
  delay(1500);
  digitalWrite(relay, LOW);
  oldTime = millis();
  while (1==1){
    doThing(vol);
    if (total >= vol) break;
  }
  lcd.clear();
}

void increase(){
  pulseCount++;
}

void wel(){
  lcd.setCursor(0, 0);
  lcd.print("***GROUP 3***");
  lcd.setCursor(0, 1);
  lcd.print("*FINAL PROJECT*");
}

void dplay(int x, int y, String s){
  lcd.setCursor(x, y);
  lcd.print(s);
}
