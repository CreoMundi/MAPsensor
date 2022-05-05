/*
  Electronic steering for a vacuum pump via a 96 753 795 80 MAP sensor:
  
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
  
  
  CURRENT ERROR WHILE AT ATM. PRESS.: -0.02 BAR (-2 kPa)
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
volatile bool running = false;      // for noticing the interrupt
float pres_set;                     // for the setPressure()
float current_pressure;

float absPresMeasure(float voltage);
float avgVoltRead(void);
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
}


void loop() {
  
  float voltage = avgVoltRead();
  current_pressure = absPresMeasure(voltage);
  
  if (running = false){
    setPressure();
  } else {
    maintainPressure();
  }
  
  Serial.println(current_pressure);
  Serial.print("\t");
  Serial.println(voltage);
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


  // measure absolute pressure in kPa
float absPresMeasure(float voltage){
  return (328.24 * voltage / INPUT_VOLTAGE - 6.53);
}


  // use potentiometer to choose pressure
void setPressure(void){
  float value;
  while(running = false){
    pres_set = fmap(analogRead(POTENTIOMETER), -0.1, 0, 0, 1023);   // show in [bar], -0.1 bar to 0 bar
    
    // show on screen
    
    if (digitalRead(ACPT_BTN), LOW){
      running = true;
    }
  }
}


  // keep the pressure within a given hysteresis
void maintainPressure(void){
  if (absPresMeasure(avgVoltRead) <= (pres_set - MIN_HYST)){
    digitalWrite(RELAY, HIGH);
  }  else if (absPresMeasure(avgVoltRead) >= (pres_set + MAX_HYST)){
     digitalWrite(RELAY, LOW);
   }
}


  // turn the pump off, ONLY FOR INTERRUPT
void reset(void){
  digitalWrite(RELAY, LOW);
  running = false;
}


float fmap(float x, float in_min, float in_max, float out_min, float out_max){
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
