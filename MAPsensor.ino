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
  MAP sensor linear model formula:
  y=ax+b; a = 328,2416; b = -6,5278
  Pabs=328,2416*Um/Us-6,5278; Pabs - absolute pressure; Um - measured voltage; Us - supply voltage
  Um is proportional to the supplied voltage, so Um is always divided by Us
  
  reference voltage from LM385Z 2.5 is used in order to increase resolution
  Ur = 2,485 V (measured with multimeter)
  
  CURRENT ERROR WHILE AT ATM. PRESS. ~ -0.02 BAR (-2 kPa)

  Created by Jan Zaslawski
*/

const int MAPSEN = A0;              // set MAP sensor input on Analog port 0
const int RELAY = 13;               // output for the relay switch
const int POTENTIOMETER = A5;
const int RST_BTN = 2;              // interrupt pin
const int ACPT_BTN = 8;
const int SAMPLES = 20;             // number of samples used for the approximation
const float MIN_HYST = 0.01;        // minimum hysteresis in bar
const float MAX_HYST = 0.01;        // maximum hysteresis in bar
const float INPUT_VOLTAGE = 4.965;  // measured with multimeter
const float REF_VOLTAGE = 2.485;    // from reference source
volatile bool pump_running = false; // for noticing the interrupt
float pres_set;                     // for the setPressure()
float P_atm;

float absPresMeasure(void);
float avgVoltRead(void);
void calibrate(void);
void setPressure(void);
void maintainPressure(void);
void reset(void);
float fmap(float x, float in_min, float in_max, float out_min, float out_max);


void setup() {
  Serial.begin(9600);
  pinMode(RST_BTN, INPUT_PULLUP);
  pinMode(ACPT_BTN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(RST_BTN), reset, LOW);
  analogReference(EXTERNAL);         // ref voltage to AREF pin
  pinMode(RELAY, OUTPUT);
  digitalWrite(RELAY, LOW);
  calibrate();
}


void loop() {
  
  if (pump_running == false){
    setPressure();
  } else {
    maintainPressure();
  }

  Serial.println(absPresMeasure());
  Serial.print("\t");
  Serial.println(avgVoltRead());
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
float absPresMeasure(void){
  float voltage = avgVoltRead();
  float pres_kpa = (328.24 * voltage / INPUT_VOLTAGE - 6.53);
  return fmap(pres_kpa, 10.0, P_atm, -0.9, 0);
}


  // use potentiometer to choose pressure
void setPressure(void){
  while(pump_running == false){
    pres_set = fmap(analogRead(POTENTIOMETER), 0, 1023, -0.9, 0.);   // show in [bar], -0.1 bar to 0 bar
    
    Serial.println(pres_set);
    Serial.print("\n");
    delay(100);
    
    if (digitalRead(ACPT_BTN) == LOW){
      pump_running = true;
    }
  }
}


  // keep the pressure within a given hysteresis
void maintainPressure(void){
  float pres = absPresMeasure();
  if (pres <= (pres_set - MIN_HYST)){
    digitalWrite(RELAY, HIGH);
  }  else if (pres >= (pres_set + MAX_HYST)){
     digitalWrite(RELAY, LOW);
  }
}


  // turn the pump off, ONLY FOR INTERRUPT
void reset(void){
  digitalWrite(RELAY, LOW);
  pump_running = false;
  
  //
  Serial.println("INTERRUPT");
  //
  
}


float fmap(float x, float in_min, float in_max, float out_min, float out_max){
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
