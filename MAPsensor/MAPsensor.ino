/*
  Simple control system for a vacuum pump via a 96 753 795 80 MAP sensor:
  
         Pinout:
         _______ 
       /  ' ' '  \
      |  -  -  -  |
       \ _o_o_o_ /
           
       out GND +5V
  
  100 kPa = 1 bar
  
  MAP sensor min pressure measured: 10 kPa with 0.25 V (typical)
  MAP sensor max pressure measured: 307.5 kPa with 4.75 V (typical)
  
  8 kPa with 0.25 V (empirical with reference voltage)
  100.89 kPa with 1.615V (empirical with reference voltage)
  
  MAP sensor linear model formula:
  y=ax+b; a = 328,2416; b = -6,5278
  reference voltage from LM385Z 2.5 is used in order to increase resolution
  Ur = 2,485 V (measured with multimeter)

  FIRST RUN MUST BE DONE @ ATMOSPHERIC PRESSURE FOR CALIBRATION

  Created by Jan Zaslawski
*/

#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  

const int MAPSEN = A0;              // set MAP sensor input on Analog port 0
const int RELAY = 5;                // output pin for the relay switch
const int POTENTIOMETER = A3;
const int RST_BTN = 2;              // interrupt pin
const int ACPT_BTN = 8;
const int SAMPLES = 20;             // number of samples used for the avgVoltRead approximation
const float INPUT_VOLTAGE = 4.965;  // measured with multimeter
const float REF_VOLTAGE = 2.485;    // from reference source
volatile bool pump_running = false; // for noticing the interrupt
float pres_set;                     // for the setPressure()
float time_set;
float low_hist;                     // hysteresis in bar
float high_hist;
float P_atm;                        // measured once while calibrating
unsigned long starting_time;

void runDebug(void)
float presMeasure(void);
float avgVoltRead(void);
void calibrate(void);
void setPressure(void);
void setHysteresis(void);
void setTimer(void);
void maintainPressure(float current_pressure);
void checkTimer(void);
void reset(void);
float fmap(float x, float in_min, float in_max, float out_min, float out_max);


void setup() { 
  lcd.begin(16,2);
  lcd.backlight();
  
  pinMode(RST_BTN, INPUT_PULLUP);
  pinMode(ACPT_BTN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(RST_BTN), reset, LOW);
  analogReference(EXTERNAL);         // ref voltage to AREF pin
  pinMode(RELAY, OUTPUT);
  digitalWrite(RELAY, LOW);
  calibrate();

//  -----------
//  runDebug();
//  -----------

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("PRAWY--------SET");
  lcd.setCursor(0,1);
  lcd.print("LEWY-------RESET");
  delay(5000);
}


void loop() {
  float current_pressure;
  
  if (pump_running == false){
    setPressure();
    setHysteresis();
    setTimer();
    pump_running = true;
  } else {
    current_pressure = presMeasure();
    maintainPressure(current_pressure);
    checkTimer();
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Cisnienie: ");
    lcd.setCursor(0,1);
    lcd.print(current_pressure);
    lcd.print(" bar");
  }
}



void runDebug(void){
  Serial.begin(9600);
  
  while(1){
    Serial.print("relative pressure:\t");
    Serial.print(presMeasure());
    Serial.print(" bar \n voltage:\t");
    Serial.print(avgVoltRead());
    Serial.println(" V \n atmospheric pressure:\t");
    Serial.print(P_atm);
    Serial.println(" kPa \n");
    delay(400);
  }
}


  // measure voltage and approximate for increased precision
float avgVoltRead(void){
  float sum = 0;
  int sample_count = 0;
  
  while (sample_count < SAMPLES) {
    sum += (float)analogRead(MAPSEN);
    sample_count++;
    delay(10);
  }
  return ((sum / SAMPLES * REF_VOLTAGE) / 1024.0);
}


  // measure the current atmospheric pressure for reference
void calibrate(void){
  float voltage = avgVoltRead();
  P_atm = (328.24 * voltage / INPUT_VOLTAGE - 6.53);
}


  // calculate absolute pressure in kPa & convert to bar
float presMeasure(void){
  float voltage = avgVoltRead();
  float pres_kpa = (328.24 * voltage / INPUT_VOLTAGE - 6.53);
  return fmap(pres_kpa, 8.0, P_atm, -0.9, 0);
}


void setPressure(void){
  bool P = false;
  while(P == false){
    pres_set = fmap(analogRead(POTENTIOMETER), 0, 1023, -0.9, 0);   // show in [bar]
        
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Ustaw cisnienie:");
    lcd.setCursor(0,1);
    lcd.print(pres_set);
    lcd.print(" bar");
    delay(200);
    
    if (digitalRead(ACPT_BTN) == LOW){
      P = true;
      delay(1000);
    }
  }
}


void setHysteresis(void){
  bool H1 = false;
  bool H2 = false;
  
  while(H1 == false){
    low_hist = map(analogRead(POTENTIOMETER), 0, 1023, 0, 20);
    low_hist = low_hist * 0.01; 

    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Dolna histereza:");
    lcd.setCursor(0,1);
    lcd.print(low_hist);
    lcd.print(" bar");
    delay(200);
    
    if (digitalRead(ACPT_BTN) == LOW){
      H1 = true;
      delay(1000);
    }
  }

    while(H2 == false){
    high_hist = map(analogRead(POTENTIOMETER), 0, 1023, 0, 20);
    high_hist = high_hist * 0.01; 

    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Gorna histereza:");
    lcd.setCursor(0,1);
    lcd.print(high_hist);
    lcd.print(" bar");
    delay(200);
    
    if (digitalRead(ACPT_BTN) == LOW){
      H2 = true;
      delay(1000);
    }
  }
}


void setTimer(void){
  bool T = false;
  while(T == false){
    time_set = map(analogRead(POTENTIOMETER), 0, 1023, 0, 96);
    time_set = time_set * 0.25; 

    if (time_set == 0){
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Praca ciagla");
      delay(200);
      
    } else {
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Wylacz za: ");
      lcd.setCursor(0,1);
      lcd.print(time_set);
      lcd.print(" h");
      delay(200);
    }
    
    if (digitalRead(ACPT_BTN) == LOW){
      T = true;
      starting_time = millis();
      delay(1000);
    }
  }
}


  // whenever pressure is higher than current_pressure + histeresis turn the pump on
void maintainPressure(float current_pressure){
  if (current_pressure >= (pres_set + high_hist)){
    digitalWrite(RELAY, HIGH);
  }
  else if (current_pressure <= (pres_set - low_hist)){
     digitalWrite(RELAY, LOW);
  }
}


void checkTimer(void){
  unsigned long time_set_to_milis = time_set * 3600000;
  if ((millis() - starting_time) >= time_set_to_milis){
    digitalWrite(RELAY, LOW);
    pump_running = false;
  }
}


  // turn the pump off, ONLY FOR INTERRUPT
void reset(void){
  digitalWrite(RELAY, LOW);
  pump_running = false;
}


  // like map function but with float
float fmap(float x, float in_min, float in_max, float out_min, float out_max){
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
