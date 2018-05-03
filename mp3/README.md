# Voice set for NOKOlino - German

There are three sets: One for the 16MBit = **2MByte** variant of the JQ6500, and one for the 8MBit = **1MByte** variant in lower quality and, brand new, a 32MBit **4 MByte** Variant with more sound files, for newer 32MBit-JQ6500 modules.  

Copy these mp3s on the JQ6500 module. Strictly observe the sequence!
021.mp3 must be copied as the 21st file. See the [Wiki](https://github.com/NikolaiRadke/NOKOlino/wiki/MP3s-aufspielen) for more 
detailed instructions.  

**Windows** users can use the (chinese) interface. See this [instructions](http://sparks.gogo.co.nz/jq6500/index.html), written by James Sleeman. **Linux** and **Mac** users should try the [uploading tool](https://github.com/NikolaiRadke/NOKOlino/tree/master/linux_uploader), written by Max Reinhard.  
  
**32MBit:**
  
| Number  | Assignment               |
| --------|-------------------------:|
| 001-020 | Response to belly button |
| 021-069 | Time based events        |
| 070     | BEEP battery warning     |
  
  **8Mbit and 16MBit:**
  
| Number  | Assignment               |
| --------|-------------------------:|
| 001-040 | Response to belly button |
| 041-163 | Time based events        |
| 164     | BEEP battery warning     |
  
The voice has been spoken by my fellow actor [Carsten Caniglia](https://filmmakers.de/carsten-caniglia). Why? 
He has done a great job - moreover I don't want to listen to my own voice every day, especially when development takes more than a year! (Ian said I sound really weird in real life. Thanks, mate.)  

### Your own voice set  
  
You want to use your own files? No problem. Look into the sketch **NOKOlino.ino**, set the definition **Set_own** and change the lines  
``` #define Button_event  0 ```  for the last voice file when the button is pressed and  
``` #define Time_event   0``` for the last voice file for the time based events.  
The *beep* for the battery warning should be the last file.  
  
Keep in your mind, that the device ist limited to **4Mbyte - 256kb**, **2MByte - 256kb** or **1MByte - 256kb**!
