# Voice set for NOKOlino - German

There are five sets: Three **male** sets, one for the 16MBit = **2MByte** variant of the JQ6500, and one for the 8MBit = **1MByte** variant in lower quality and a 32MBit **4 MByte** variant with more sound files, for newer 32MBit-JQ6500 modules. The fourth set is a 32MBit **4 MByte** **female** variant, brand new for *NOKOlina* and the last set ist the 16MBit **2 MByte** variant.    
  
Copy these mp3s on the JQ6500 module. Strictly observe the sequence!
021.mp3 must be copied as the 21st file. See the [Wiki](https://github.com/NikolaiRadke/NOKOlino/wiki/MP3s-aufspielen) for more detailed instructions.  
  
**Windows** users can use the (chinese) interface. See this [instructions](http://sparks.gogo.co.nz/jq6500/index.html), written by James Sleeman. **Linux** and **Mac** users should try the [uploading tool](https://github.com/NikolaiRadke/NOKOlino/tree/master/linux_uploader), written by Max Reinhard.  
    
 **16MBit:**  
  
| Number  | Assignment               |
| --------|-------------------------:|
| 001-030 | Response to belly button |
| 031-079 | Time based events        |
| 080     | BEEP battery warning     |
  
 **32MBit:**  
  
| Number  | Assignment               |
| --------|-------------------------:|
| 001-040 | Response to belly button |
| 041-163 | Time based events        |
| 164     | BEEP battery warning     |
  
The male voice has been spoken by my fellow actor [Carsten Caniglia](https://filmmakers.de/carsten-caniglia). Why? 
He has done a great job - moreover I don't want to listen to my own voice every day, especially when development takes more than a year! (Ian said I sound really weird in real life. Thanks, mate.). The female voice was spoken by my fellow actress [Norma Anthes](http://norma-anthes.de), thank you, lady!    

### Your own voice set  
  
You want to use your own files? No problem. Look into the sketch **NOKOlino.ino**, set the definition **Set_own** and change the lines  
``` #define Button_event  0 ```  for the last voice file when the button is pressed and  
``` #define Time_event   0``` for the last voice file for the time based events.  
The *beep* for the battery warning should be the last file.  
  
Keep in your mind, that the device ist limited to **4Mbyte - 256kb**, **2MByte - 256kb** or **1MByte - 256kb**!
