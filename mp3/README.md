# Voice set for NOKOlino - German

There are to sets: One for the (strongly recommended) 16MBit = **2MByte** variant of the JQ6500, and one for the 8MBit = **1MByte** variant in lower quality.  

Copy these mp3s on the JQ6500 module. Strictly observe the sequence!
021.mp3 must be copied as the 21st file. See the [Wiki](https://github.com/NikolaiRadke/NOKOlino/wiki/MP3s-aufspielen) for more 
detailed instructions.  

**Windows** users can use the (chinese) interface. See this [instructions](http://sparks.gogo.co.nz/jq6500/index.html), written by James Sleeman. **Linux** and **Mac** users should try the [uploading tool](https://github.com/NikolaiRadke/NOKOlino/tree/master/linux_uploader), written by Max Reinhard.  
  
| Number  | Assignment               |
| --------|-------------------------:|
| 001-020 | Response to belly button |
| 021-069 | Time based events        |
| 70      | BEEP battery warning     |
  
The voice has been spoken by my fellow actor [Carsten Caniglia](http://www.carstencaniglia.com). Why? 
He has done a great job - moreover I don't want to listen to my own voice every day, especially when development takes more than a year! (Ian said I sound really weird in real life. Thanks, mate.)  

### Your own voice set  
  
You want to use your own files? No problem. Look into the sketch **NOKOlino.ino** and change the lines  
``` #define Button_event  20 ```  for the last voice file (20.mp3 in this case) when the button is pressed and  
``` #define Time_event   69``` for the last voice file (69.mp3 in this case) for the time based events.  
The *beep* for the battery warning should be the last file (70.mp3 in this case).  
  
Keep in your mind, that the device ist limited to **2MByte - 256kb** or **1MByte - 256kb**!
