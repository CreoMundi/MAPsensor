// Electronic steering for a vacuum pump via a 96 753 795 80 MAP sensor:
// 
//       Pinout:   
//       _______ 
//     /  ' ' '  \
//    |  -  -  -  |
//     \ _o_o_o_ /
//         
//     out GND +5V
//
// 100 kPa = 1 bar
//
// MAP sensor min pressure measured: 10 kPa with 0.25 V (typical)
// MAP sensor max pressure measured: 307.5 kPa with 4.75 V (typical)
// MAP sensor linear model formula:
// y=ax+b; a = 328,2416; b = -6,5278
// Pabs=328,2416*Um/Us-6,5278; Pabs - absolute pressure; Um - measured voltage; Us - supply voltage
// Um is proportional to the supplied voltage, so Um is always divided by Us
//
// reference voltage from LM385Z 2.5 is used in order to increase resolution
// Ur = 2,485 V (measured with multimeter)
//
//
// CURRENT ERROR WHILE AT ATM. PRESS.: -0.02 BAR (-2 kPa)


const int MAPSEN = A0;              // set MAP sensor input on Analog port 0
const int RELAY = 13;               // output for the relay switch
const int SAMPLES = 20;             // number of samples for the approximation
const float INPUT_VOLTAGE = 4.965;  // measured with multimeter
const float REF_VOLTAGE = 2.485;    // from reference source

float absolutePressure(float voltage);
float avgVoltRead(void);

// unsigned long time_now;

void setup() {
  Serial.begin(9600);
  analogReference(EXTERNAL);         // ref voltage to AREF pin
  pinMode(RELAY, OUTPUT);
  digitalWrite(RELAY, LOW);
}


void loop() { 
  float voltage = avgVoltRead();
  float Pabs = absolutePressure(voltage);
  Serial.println(Pabs);
  Serial.print("\t");
  Serial.println(voltage);
}


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

float absolutePressure(float voltage){              //absolute pressure in kPa
  return (328.24 * voltage / INPUT_VOLTAGE - 6.52);
}
