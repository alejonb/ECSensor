
//*********** Converting to ppm [Learn to use EC it is much better**************//
// Hana      [USA]        PPMconverion:  0.5
// Eutech    [EU]          PPMconversion:  0.64
//Tranchen  [Australia]  PPMconversion:  0.7
"PPMconversion=0.5;"

//*************Compensating for temperature ************************************//
//The value below will change depending on what chemical solution we are measuring
//0.019 is generaly considered the standard for plant nutrients [google "Temperature compensation EC" for more info
float TemperatureCoef = 0.019; //this changes depending on what chemical we are measuring
 
//********************** Cell Constant For Ec Measurements *********************//
//Mine was around 2.9 with plugs being a standard size they should all be around the same
//But If you get bad readings you can use the calibration script and fluid to get a better estimate for K
float K=2.45;

### REF
https://create.arduino.cc/projecthub/mircemk/arduino-electrical-conductivity-ec-ppm-tds-meter-c48201
https://hackaday.io/project/7008-hacking-the-way-to-growing-food/log/24646-three-dollar-ec-ppm-meter-arduino 