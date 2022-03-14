
### Circuit.
#### Elements.
* DS18820 Temperature sensor by contact. Preffer with probe.
* TM1637 4-digits arduino display sensor.
* Arduino nano 
### Considerations 
#### Converting to ppm
* Hana      [USA]        PPMconverion:  0.5
* Eutech    [EU]          PPMconversion:  0.64
* Tranchen  [Australia]  PPMconversion:  0.7

#### Compensating for temperature:

The value below will change depending on what chemical solution we are measuring 0.019 is generaly considered the standard for plant nutrients.

 
#### Cell Constant For Ec Measurements
float K=2.45; ## The last time i calibrate with hana buffer

### REF
https://create.arduino.cc/projecthub/mircemk/arduino-electrical-conductivity-ec-ppm-tds-meter-c48201
https://hackaday.io/project/7008-hacking-the-way-to-growing-food/log/24646-three-dollar-ec-ppm-meter-arduino 
https://www.aqion.de/site/112  [google "Temperature compensation EC" for more info]
https://www.servovendi.com/uk/catalog/product/view/id/489/s/ec-buffer-solution-calibration-hanna-1413-s-cm-20ml-hi-70031/category/773/ 
