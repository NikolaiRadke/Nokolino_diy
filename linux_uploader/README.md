# Uploading files on the JQ6500 with Linux

It is quite easy with this command line tool, written by [Reinhard Max](https://chiselapp.com/user/rmax/repository/jq6500).  I added a precompiled version (64 Bit). It is free, but it is also *beerware*... ;-)  

1. Check, if ``` ./jq6500 ``` is running.  
2. If not, compile it by yourself, just by typing ``` make ```. 
3. Try again.
4. Connect your JQ6500 module to your system with a micro USB cable.  
5. Test your device by typing  ``` ./jq6500 -S ```. The tool will tell you the detected flash size.  
6. Upload your files by typing   
  ``` ./jq6500 -W ../mp3/male/mp3_8MBit/* ```(1 MByte devices)  or  
  ``` ./jq6500 -W ../mp3/male/mp3_16MBit/* ``` (2 MByte devices) or  
  ``` ./jq6500 -W ../mp3/male/mp3_32MBit/* ``` (4 MByte devices) or  
  ``` ./jq6500 -W ../mp3/female/mp3_16MBit/* ``` (2 MByte devices, female voice set) or  
  ``` ./jq6500 -W ../mp3/female/mp3_32MBit/* ``` (4 MByte devices, female voice set).    
  
If your MP3 files are in another directory, modify the last argument.  

## Repairing a JQ6500 without Windows software
  
Strangely, some modules come without the integrated Windows software. If you are fond of nice Chinese frontends or have a friend in need without Linux, the tool can fix the module.

1. Like above, check if the tool is running correctly.  
2. Connect your JQ6500 module to your system with a micro USB cable.  
3. Just type ``` make flash ```. 

That's it. You can now use your module with Windows.  

