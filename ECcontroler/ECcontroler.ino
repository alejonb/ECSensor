 
#include <OneWire.h>
#include <DallasTemperature.h>
 
#include <TM1637.h>

#define M_EC_UP 11
#define M_EC_UP_SPEED 130
#define ZERO_SPEED 0

int CLK = 2;
int DIO = 3;

TM1637 tm(CLK,DIO);
 

//************************* User Defined Variables ********************************************************//
 
 
//##################################################################################
//-----------  Do not Replace R1 with a resistor lower than 300 ohms    ------------
//##################################################################################
 
int R1= 1000;
int Ra=25; //Resistance of powering Pins
int ECPin= A0;
int ECGround=A1;
int ECPower =A4;

float PPMconversion=0.5;  // Hana      [USA]    
 
//*************Compensating for temperature ************************************//
//The value below will change depending on what chemical solution we are measuring
//0.019 is generaly considered the standard for plant nutrients [google "Temperature compensation EC" for more info
float TemperatureCoef = 0.019; //this changes depending on what chemical we are measuring
 
 //********************** Cell Constant For Ec Measurements *********************//
//Mine was around 2.9 with plugs being a standard size they should all be around the same
//But If you get bad readings you can use the calibration script and fluid to get a better estimate for K
float K=2.45;
   
//************ Temp Probe Related *********************************************//
#define ONE_WIRE_BUS 10          // Data wire For Temp Probe is plugged into pin 10 on the Arduino
const int TempProbePossitive =8;  //Temp Probe power connected to pin 9
const int TempProbeNegative=9;    //Temp Probe Negative connected to pin 8
  
 
//***************************** END Of Recomended User Inputs *****************************************************************//
 
 
OneWire oneWire(ONE_WIRE_BUS);// Setup a oneWire instance to communicate with any OneWire devices
DallasTemperature sensors(&oneWire);// Pass our oneWire reference to Dallas Temperature.
 
 
float Temperature=10;
float EC=0;
float EC25 =0;
int ppm =0;
 
 
float raw= 0;
float Vin= 5;
float Vdrop= 0;
float Rc= 0;
float buffer=0;  
 
//*********************************Setup - runs Once and sets pins etc ******************************************************//
void setup()
{
  Serial.begin(9600);
  pinMode(TempProbeNegative , OUTPUT ); //seting ground pin as output for tmp probe
  digitalWrite(TempProbeNegative , LOW );//Seting it to ground so it can sink current
  pinMode(TempProbePossitive , OUTPUT );//ditto but for positive
  digitalWrite(TempProbePossitive , HIGH );
  pinMode(ECPin,INPUT);
  pinMode(ECPower,OUTPUT);//Setting pin for sourcing current
  pinMode(ECGround,OUTPUT);//setting pin for sinking current
  digitalWrite(ECGround,LOW);//We can leave the ground connected permanantly

  /** Motor **/  
  pinMode(M_EC_UP, OUTPUT);
  
  delay(100);// gives sensor time to settle
  sensors.begin();
  delay(100);
  //** Adding Digital Pin Resistance to [25 ohm] to the static Resistor *********//
  // Consule Read-Me for Why, or just accept it as true
  R1=(R1+Ra);// Taking into acount Powering Pin Resitance
  /** Display **/
  Serial.println("Measurments at 5's Second intervals [Dont read Ec morre than once every 5 seconds]:");
  tm.init();
  tm.set(2);
 
};
//******************************************* End of Setup **********************************************************************//
 

//************************************* Main Loop - Runs Forever ***************************************************************//
//Moved Heavy Work To subroutines so you can call them from main loop without cluttering the main loop
void loop()
{ 
   
  GetEC();          //Calls Code to Go into GetEC() Loop [Below Main Loop] dont call this more that 1/5 hhz [once every five seconds] or you will polarise the water
  PrintReadings();  // Cals Print routine [below main loop]
   
  //displayNumber((int)(EC25*1000));
  displayNumber((int)(ppm));
   
  delay(5000);  
 
}
//************************************** End Of Main Loop **********************************************************************//
 
void ecUp() {
    analogWrite(M_EC_UP, M_EC_UP_SPEED);
    delay(DROP_TIME);
    analogWrite(M_EC_UP, ZERO_SPEED);
}
 
//************ This Loop Is called From Main Loop************************//  
 
void displayNumber(int num){   
  Serial.println(num);
  tm.display(3, num % 10);   
  tm.display(2, num / 10 % 10);   
  tm.display(1, num / 100 % 10);   
  tm.display(0, num / 1000 % 10);
}
 
void GetEC(){ 
   
  //*********Reading Temperature Of Solution *******************//
  sensors.requestTemperatures();// Send the command to get temperatures
  Temperature=sensors.getTempCByIndex(0); //Stores Value in Variable   
  //************Estimates Resistance of Liquid ****************//
  digitalWrite(ECPower,HIGH);
  raw= analogRead(ECPin);
  raw= analogRead(ECPin);// This is not a mistake, First reading will be low beause if charged a capacitor
  digitalWrite(ECPower,LOW);   
  //***************** Converts to EC **************************//
  Vdrop= (Vin*raw)/1024.0;
  Rc=(Vdrop*R1)/(Vin-Vdrop);
  Rc=Rc-Ra; //acounting for Digital Pin Resitance
  EC = 1000/(Rc*K);  
  //*************Compensating For Temperaure********************//
  EC25  =  EC/ (1+ TemperatureCoef*(Temperature-25.0));
  ppm=(EC25)*(PPMconversion*1000);
   
}
//************************** End OF EC Function ***************************//
 
 
 
 
