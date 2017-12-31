# ![NOKO](http://www.nikolairadke.de/NOKOlino/nokolino_klein.png) NOKOlino

The ATTiny-based litte brother of NOKO. Tiny and cute. And everyone is invited to **contribute**. NOKOlino is for everyone. What's new? See the [newsblog](https://github.com/NikolaiRadke/NOKOlino/tree/master/NEWS.md).  

**NOTE:** This repo is under construction and will be finished in a couple of days. Stay tuned! --31.12.2017

![NOKOlino light blue](http://www.nikolairadke.de/NOKOlino/nokolino_wiki.png)
  
Like his big brother NOKO, the little NOKOlino delights it's surroundings with cheeky comments. But unlike NOKO he is more obstinate - no radio, a display, no frills. Only one on-off switch and a big button, but he is small, can stay active for days and is, in contrast to his big brother, very easy to build.  

**NOKOlino** stands for **NO**ra and Ni**KO**s monster - the appendix **lino** describes the small version.  
  
### Notes for non-German NOKOlino makers  
  
Right now, the building wiki, the voice set and the manual are in German. I'll try to translate the wiki and the manual on day. Until then, feel free to ask me for translation, if you need help. **Contribute!** I need an English voice set and English text files. In addition, any other language is welcome, NOKO is to be cosmopolitan!
  
### Prerequisites
  
NOKO needs Arduino IDE 1.6.6 -or newer. (https://www.arduino.cc/en/Main/Software) Copy the folder `NOKO/src/[language]/NOKO/` into your sketch folder and install ATtiny support. See [how to compile](https://github.com/NikolaiRadke/NOKOlino/tree/master/howto_compile) for further instructions. 
  
### Content

```
NOKO/
├── howto_compile/
|   Manual how compile the sketches in your Arduino IDE and precompiled sketched  
|   └── precompiled/
|       Precompiled hex-files
|       ├── ATtiny45/
|       |   Versions for the ATtiny45.
|       └── ATtiny85/
|           Versions for the ATtiny85.
|
├── linux_uploader/
|   Tool for uploading MP3-files to the JQ6500.
|
├── manual/
|   NOKO manual in PDF format and a template for a packaging desin.
|   └── scribus/
|       Source documents for Scribus with all illustrations and fonts.
|
├── mp3/
|   Voiceset for NOKOlino in German. English needed? Contribute!
|   ├── mp3_8MBit/
|   |   Low qualitiy files for the 1MB-JQ6500.
|   └── mp3_16MBit/
|       High quality files for the 2MB-JQ6500.
|
├── schematics/
|   The Schematics for the NOKOlino parts, the NOKO sewing pattern and shopping lists.
|   └── geda/
|       Source documents for gEDA.
|
└── src/
    ATtiny sketches
    ├── NOKOlino/
    |   Standard version. Full featured. Take this one.
    └── NOKOlino_easy/
        Easy-to-understand learing version. 
```
### Let's get started!

Interested in NOKO? Great! Let's start here:  
[German wiki](https://github.com/NikolaiRadke/NOKOlino/wiki).  
English wiki will follow. One day...
