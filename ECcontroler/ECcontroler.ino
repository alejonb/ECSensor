 
#include <OneWire.h>
#include <DallasTemperature.h>
 
#include <TM1637.h>
#include <Arduino_JSON.h>

#define M_EC_UP 11
#define M_EC_UP_SPEED 130
#define ZERO_SPEED 0
#define POT_PIN A2
#define DROP_TIME 100

//************************* User Defined Variables ********************************************************//
  

#define EC_PIN A0
#define EC_GND A1
#define EC_POWER A4

#define PPM_FACTOR 0.5  // Hana      [USA]    
#define TEMP_COEF 0.019

 //********************** Cell Constant For Ec Measurements *********************//
#define K 2.45

 //##################################################################################
//-----------  Do not Replace R1 with a resistor lower than 300 ohms    ------------
//##################################################################################
 
int R1 = 1000;
int Ra = 25; //Resistance of powering Pins  

//************ Temp Probe Related *********************************************//
#define ONE_WIRE_BUS 10          // Data wire For Temp Probe is plugged into pin 10 on the Arduino
const int TempProbePossitive =8;  //Temp Probe power connected to pin 9
const int TempProbeNegative=9;    //Temp Probe Negative connected to pin 8
  
 
//***************************** END Of Recomended User Inputs *****************************************************************//
 
// Displays
TM1637 ECDisplay(2,3);
TM1637 desirdedECDisplay(4,5); 
 
OneWire oneWire(ONE_WIRE_BUS);// Setup a oneWire instance to communicate with any OneWire devices
DallasTemperature sensors(&oneWire);// Pass our oneWire reference to Dallas Temperature.
 
 
float Temperature=10;
float EC=0;
float EC25 =0;
int ppm =0;
boolean manualMode = true;
JSONVar Data;
 
 
float raw= 0;
float Vin= 5;
float Vdrop= 0;
float Rc= 0;
float buffer=0;  
float delay_timer = 0;
 
//*********************************Setup - runs Once and sets pins etc ******************************************************//
void setup()
{
  Serial.begin(9600);
  
  pinMode(TempProbeNegative , OUTPUT ); //seting ground pin as output for tmp probe
  digitalWrite(TempProbeNegative , LOW );//Seting it to ground so it can sink current
  pinMode(TempProbePossitive , OUTPUT );//ditto but for positive
  digitalWrite(TempProbePossitive , HIGH );
  
  pinMode(EC_PIN,INPUT);
  pinMode(EC_POWER,OUTPUT);//Setting pin for sourcing current
  pinMode(EC_GND,OUTPUT);//setting pin for sinking current
  digitalWrite(EC_GND,LOW);//We can leave the ground connected permanantly
  
  delay(100);// gives sensor time to settle
  sensors.begin();
  delay(100);
  //** Adding Digital Pin Resistance to [25 ohm] to the static Resistor *********//
  // Consule Read-Me for Why, or just accept it as true
  R1=(R1+Ra);// Taking into acount Powering Pin Resitance

  //Serial.println("Measurments at 5's Second intervals [Dont read Ec morre than once every 5 seconds]:");
  
  /** Motor **/  
  pinMode(M_EC_UP, OUTPUT);
  
  /** Displays **/
  ECDisplay.init();
  ECDisplay.set(2);

  desirdedECDisplay.init();
  desirdedECDisplay.set(2);
  pinMode(POT_PIN, INPUT);
  delay_timer = -10000;
}
//******************************************* End of Setup **********************************************************************//
 

//************************************* Main Loop - Runs Forever ***************************************************************//
//Moved Heavy Work To subroutines so you can call them from main loop without cluttering the main loop
void loop()
{ 
  if(millis()-delay_timer>5000){    
    delay_timer = millis();
    GetEC();          //Calls Code to Go into GetEC() Loop [Below Main Loop] dont call this more that 1/5 hhz [once every five seconds] or you will polarise the water
    //displayEC((int)(EC25*1000));
    displayValue((int)abs(ppm),"sense");
  }else{
    displayValue((int)( get_desired_EC()),"desired");    
    wait_for_command();
  } 
  delay(100);  
 
}
//************************************** End Of Main Loop **********************************************************************//

void wait_for_command(){
  if (Serial.available() > 0)
  {
    
    String msg = Serial.readString(); 
    JSONVar myObject = JSON.parse(msg);
    String command = "NONE";
    command = myObject["COMMAND"];
    if(command.equals("ECREAD")){      
      Data["PPM"] = ppm;
      Data["Temperature"] = Temperature;
      Data["DESIRED_PPM"] = get_desired_EC();
      Data["ACK"] = "DONE";  
      Data["MSG"] = msg;
    }else if(command.equals("ECUP")){
      ecUp();   
      Data["MSG"] = msg;
      Data["ACK"] = "DONE";             
    }else{        
      Data["ACK"] = "ERROR";
      Data["MSG"] = msg;
    }
    
    Serial.println(JSON.stringify(Data));   
  }
}

void ecUp() {
    analogWrite(M_EC_UP, M_EC_UP_SPEED);
    delay(DROP_TIME);
    analogWrite(M_EC_UP, ZERO_SPEED);
}

float get_desired_EC() {
  return map(analogRead(POT_PIN), 0, 1023, 900, 1300) ;
}
//************ This Loop Is called From Main Loop************************//  
 
void displayValue(int num,String type){   
  if(type.equals("sense")){

    ECDisplay.display(3, num % 10);   
    ECDisplay.display(2, num / 10 % 10);   
    ECDisplay.display(1, num / 100 % 10);   
    ECDisplay.display(0, num / 1000 % 10);
  }else{
    desirdedECDisplay.display(3, num % 10);   
    desirdedECDisplay.display(2, num / 10 % 10);   
    desirdedECDisplay.display(1, num / 100 % 10);   
    desirdedECDisplay.display(0, num / 1000 % 10);

  }
}
 
void GetEC(){ 
   
  //*********Reading Temperature Of Solution *******************//
  sensors.requestTemperatures();// Send the command to get temperatures
  Temperature=sensors.getTempCByIndex(0); //Stores Value in Variable   
  //************Estimates Resistance of Liquid ****************//
  digitalWrite(EC_POWER,HIGH);
  raw= analogRead(EC_PIN);
  raw= analogRead(EC_PIN);// This is not a mistake, First reading will be low beause if charged a capacitor
  digitalWrite(EC_POWER,LOW);   
  //***************** Converts to EC **************************//
  Vdrop= (Vin*raw)/1024.0;
  Rc=(Vdrop*R1)/(Vin-Vdrop);
  Rc=Rc-Ra; //acounting for Digital Pin Resitance
  EC = 1000/(Rc*K);  
  //*************Compensating For Temperaure********************//
  EC25  =  EC/ (1+ TEMP_COEF*(Temperature-25.0));
  ppm=(EC25)*(PPM_FACTOR*1000);
   
}
//************************** End OF EC Function ***************************//
 
 
 
 
