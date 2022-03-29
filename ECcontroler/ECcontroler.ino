
#include <OneWire.h>
#include <DallasTemperature.h>

#include <TM1637.h>
#include <Arduino_JSON.h>

#define M_EC_UP 11
#define M_EC_UP_SPEED 230
#define ZERO_SPEED 0

#define POT_PIN A2
#define DROP_TIME 1000

// Controller constants
#define ERR_MARGIN 50
#define STABILIZATION_MARGIN 0.1

#define MINUTE 1000L * 60
#define STABILIZATION_TIME 1 * MINUTE

#define SLEEPING_TIME 100
// EC Sensor conf

#define EC_PIN A0
#define EC_GND A1
#define EC_POWER A4

#define PPM_FACTOR 0.5 // Hana      [USA]
#define TEMP_COEF 0.019

//Cell Constant For Ec Measurements
#define K 2.45

//##################################################################################
//-----------  Do not Replace R1 with a resistor lower than 300 ohms    ------------
//##################################################################################

int R1 = 1000;
int Ra = 25; //Resistance of powering Pins

//************ Temp Probe Related *********************************************//
#define ONE_WIRE_BUS 10           // Data wire For Temp Probe is plugged into pin 10 on the Arduino
const int TempProbePossitive = 8; //Temp Probe power connected to pin 9
const int TempProbeNegative = 9;  //Temp Probe Negative connected to pin 8

//***************************** END Of Recomended User Inputs *****************************************************************//

// Displays
TM1637 ECDisplay(2, 3);
TM1637 desirdedECDisplay(4, 5);

OneWire oneWire(ONE_WIRE_BUS);       // Setup a oneWire instance to communicate with any OneWire devices
DallasTemperature sensors(&oneWire); // Pass our oneWire reference to Dallas Temperature.

float Temperature = 10;
float EC = 0;
float EC25 = 0;
int ppm = 0;
int desiredPPM = 0;
int error = 0;

boolean manualMode = false;
JSONVar Data;

float raw = 0;
float Vin = 5;
float Vdrop = 0;
float Rc = 0;
float buffer = 0;
float delay_timer = 0;

//*********************************Setup - runs Once and sets pins etc ******************************************************//
void setup()
{
  Serial.begin(9600);

  pinMode(TempProbeNegative, OUTPUT);   //seting ground pin as output for tmp probe
  digitalWrite(TempProbeNegative, LOW); //Seting it to ground so it can sink current
  pinMode(TempProbePossitive, OUTPUT);  //ditto but for positive
  digitalWrite(TempProbePossitive, HIGH);

  pinMode(EC_PIN, INPUT);
  pinMode(EC_POWER, OUTPUT); //Setting pin for sourcing current
  pinMode(EC_GND, OUTPUT);   //setting pin for sinking current
  digitalWrite(EC_GND, LOW); //We can leave the ground connected permanantly

  delay(100); // gives sensor time to settle
  sensors.begin();
  delay(100);
  //** Adding Digital Pin Resistance to [25 ohm] to the static Resistor *********//
  // Consule Read-Me for Why, or just accept it as true
  R1 = (R1 + Ra); // Taking into acount Powering Pin Resitance

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

  //displayEC((int)(EC25*1000));

  ppm = GetEC();
  desiredPPM = get_desired_EC();
  displayValue(ppm, "sense");
  displayValue(desiredPPM, "desired");
  check_for_command();

  if (!manualMode)
  {
    error = desiredPPM - ppm;

    if (error > ERR_MARGIN)
    {
      do
      {
        if (error > 0)
        {
          ecUp(DROP_TIME);
          Serial.println("Going UP!");
        }

        long start = millis();
        while (millis() - start < STABILIZATION_TIME)
        {
          delay(100);

          ppm = GetEC();
          desiredPPM = get_desired_EC();
          displayValue(ppm, "sense");
          displayValue(desiredPPM, "desired");
          Serial.print("ppm: ");
          Serial.print(ppm);
          Serial.print("\t\tdesiredPPM: ");
          Serial.println(desiredPPM);

          error = desiredPPM - ppm;
          check_for_command();
        }
      } while (abs(error) > STABILIZATION_MARGIN);
    }
  }
  delay(SLEEPING_TIME);
}
//************************************** End Of Main Loop **********************************************************************//

void check_for_command()
{
  if (Serial.available() > 0)
  {

    String msg = Serial.readString();
    JSONVar myObject = JSON.parse(msg);
    String command = "NONE";
    command = myObject["COMMAND"];
    if (command.equals("ECREAD"))
    {
      Data["PPM"] = GetEC();
      Data["Temperature"] = Temperature;
      Data["DESIRED_PPM"] = get_desired_EC();
      Data["ACK"] = "DONE";
      Data["MSG"] = msg;
    }
    else if (command.equals("ECUP"))
    {
      int dropTime = myObject["DROP_TIME"];
      ecUp(dropTime);
      Data["MSG"] = msg;
      Data["ACK"] = "DONE";
    }
    else if (command.equals("AUTO"))
    {
      manualMode = false;
      Data["MSG"] = msg;
      Data["ACK"] = "DONE";
    }
    else if (command.equals("MANUAL"))
    {
      manualMode = true;
      Data["MSG"] = msg;
      Data["ACK"] = "DONE";
    }
    else
    {
      Data["ACK"] = "ERROR";
      Data["MSG"] = msg;
    }

    Serial.println(JSON.stringify(Data));
  }
}

void ecUp(int dropTime)
{ //M_EC_UP_SPEED
  analogWrite(M_EC_UP, M_EC_UP_SPEED);
  delay(dropTime);
  analogWrite(M_EC_UP, ZERO_SPEED);
}

int get_desired_EC()
{
  return map(analogRead(POT_PIN), 0, 1023, 900, 1300);
}
//************ This Loop Is called From Main Loop************************//

void displayValue(int num, String type)
{
  if (type.equals("sense"))
  {

    ECDisplay.display(3, num % 10);
    ECDisplay.display(2, num / 10 % 10);
    ECDisplay.display(1, num / 100 % 10);
    ECDisplay.display(0, num / 1000 % 10);
  }
  else
  {
    desirdedECDisplay.display(3, num % 10);
    desirdedECDisplay.display(2, num / 10 % 10);
    desirdedECDisplay.display(1, num / 100 % 10);
    desirdedECDisplay.display(0, num / 1000 % 10);
  }
}

int GetEC()
{
  //Calls Code to Go into GetEC() Loop [Below Main Loop] dont call this more that 1/5 hhz [once every five seconds] or you will polarise the water
  if (millis() - delay_timer > 5000)
  {

    delay_timer = millis();
    //*********Reading Temperature Of Solution *******************//
    sensors.requestTemperatures();            // Send the command to get temperatures
    Temperature = sensors.getTempCByIndex(0); //Stores Value in Variable
    //************Estimates Resistance of Liquid ****************//
    digitalWrite(EC_POWER, HIGH);
    raw = analogRead(EC_PIN);
    raw = analogRead(EC_PIN); // This is not a mistake, First reading will be low beause if charged a capacitor
    digitalWrite(EC_POWER, LOW);
    //***************** Converts to EC **************************//
    Vdrop = (Vin * raw) / 1024.0;
    Rc = (Vdrop * R1) / (Vin - Vdrop);
    Rc = Rc - Ra; //acounting for Digital Pin Resitance
    EC = 1000 / (Rc * K);
    //*************Compensating For Temperaure********************//
    EC25 = EC / (1 + TEMP_COEF * (Temperature - 25.0));
    ppm = (EC25) * (PPM_FACTOR * 1000);
  }
  return ppm;
}
//************************** End OF EC Function ***************************//
