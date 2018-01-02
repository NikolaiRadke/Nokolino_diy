# Compiling instructions for NOKOlino

In order to flash your ATtiny with the NOKOlino-sketch, you need the **Arduino-IDE** (1.6.6 odr higher), with installed **ATtiny-Support** and an **Arduino** connected to your microcontroller.

### Preparing the IDE  
 
1. Open ``` File > Preferences ```.  
2. Find the field ``` Additional Boards Manager URLs ```.  
3. Enter ``` https://raw.githubusercontent.com/damellis/attiny/ide-1.6.x-boards-manager/package_damellis_attiny_index.json ```.  
4. Press ``` OK ```.  
5. Open ``` Tools > Board > Boards Manager ```.  
6. Select ``` attiny by David A. Mellis ```.  
7. Press ``` Install ```.

### Peparing your Arduino  

1. Connect your Arduino to your System.  
2. Open ``` File > Examples > ArduinoISP > ArduinoISP ```.  
3. Upload the sketch.

## Connecting your ATtiny with the Arduino  

1. Disconnect your Arduino.
2. Connect your ATtiny as seen below:  

![ATTINY-ARDUINO](http://www.nikolairadke.de/NOKOlino/attiny_steckplatine2.png)  
  

  

