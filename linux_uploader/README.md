# Uploading Files on the JQ6500 with Linux

It is quite easy with this command line tool, written by [Reinhard Max](https://chiselapp.com/user/rmax/repository/jq6500).  I added a precompiled version (64 Bit). It is free, but it is also *beerware*... ;-)  

1. Check, if ``` ./jq6500 ``` is running.  
2. If not, compile it by yourself, just by typing ``` make ```. 
3. Try again.
4. Connect your JQ6500 module to your system with a micro USB cable.  
5. Upload your files by typing   
  ``` ./jq6500 ../mp3/male/mp3_8MBit/* ```(1 MByte devices)  or  
  ``` ./jq6500 -f ../mp3/male/mp3_8MBit/* ``` (2 MByte devices) or  
  ``` ./jq6500 ../mp3/male/mp3_32MBit/* ``` (4 MByte devices) or  
  ``` ./jq6500 ../mp3/demale/mp3_8MBit/* ``` (4 MByte devices, female voice set).    
  
If your MP3 files are in another directory, modify the last argument.  

