IOTBlindsControl v1.3.0

IOTBlindsControl - Smarthome controller for roller blinds
================ = ========= ========== === ====== ======

History:
--------
About 10 years ago I built a rollerblind controller with an Arduino Nano and
433MHz receiver to be able to control it remotely. Now I wanted better inte-
gration with my domotics system and modified the code for an ESP32 chip
with WiFi and MQTT for controlling the blind.

I'd rewrite a large part of the code as I thought I little bit differently
about coding in those days.

For the first project I included a light sensor and temperature sensor to be
able to control the blinds on a sunny or hot day. I kept those in because those
are still useful.

Hardware:
---------
- Manual control: 2 push buttons with LED (red and green)
- Blind control: 2 relays. One relay is used for up or down selection
                 (changeover contact) and the second one to switch power to the
                 blinds on or off. I use an ULN2003 to drive the (in my case 5V)
                 relays from the 3V3 output of the ESP. I might add schematics
                 later.
- Light sensor: Simple analog sensor from ali-express, with digital threshold
                signal.
- Temperature sensor: Dallas DS18S20 one wire sensor.
- 433MHz receiver: Not required anymore
- Microcontroller: In my case an ESP32-S2, but another ESP32 or even ESP8266
                   will also work with some small code modifications.

Functions:
----------
- Settings are stored in EEPROM. Lots of settings can be modified.
- Webinterface for controlling the blind, reading values, logging and read/
  modify settings (IOTblindscontrol.local by default but can be changed).
- Setup as access point if network cannot be found or logged in.
- In access point mode, captive portal (website) is loaded to select network
  from and/ or modify settings.
- MQTT access (up/ down and percentage settings) and sensors readout.
- NTP clock to do time related stuff.
- Sync mode op upper and lower point to assure blind is totally up or down.
- Low power: Implementation of auto modem sleep and 80 MHz CPU frequency.
             Current is about 30mA in idle. Other sleep modes are not
             possible as the system should listen to commands from WiFi.
             It is not assumed to be a battery powered system.
- Auto down/ up on light levels.
- Auto down/ up on temperature levels.
- Auto down/ up on specific times when communication with backend fails.
- Setup for blind by open/ close time.
- MQTT auto discovery for home asistant if enabled.

Installation:
-------------
- Clone from github or download zip and unpack it
- Open IOTBlindsControl in Arduino IDE (I used Arduino IDE 2.0.3)
- Select correct microcontroller (howto's can be found online)
- Build and download code

If you upload the code, you can update to a new version via the web interface
(OTA = Over The Air). Just download the bin file in the bin folder, select it
and press the upload button. No USB connection required.

You can view/ store logging over UDP. Install udplogger to view logging live or
store logging in the background 

That's all for now ...

Please send Comments and Bugreports to hellyrulez@home.nl
