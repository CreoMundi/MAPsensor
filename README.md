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
