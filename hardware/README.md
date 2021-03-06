Here's the list of parts I used for this project:

SunAirPlus Solar Battery Controller

![SunAirPlus](sdl.jpg)

http://store.switchdoc.com/sunairplus-solar-controller-charger-sun-tracker-data-gathering-grove-header/

Solar Cell (Australian supplier - find a local equivalent!) It has to be a 6V cell. If it is to be able to charge the battery and run the ESP8266, it should produce about 500ma (3W) or more.

http://www.ebay.com.au/itm/6V-4-5W-520mA-Mini-Solar-Panel-Small-Cell-PV-Module-Charger-DIY-Tool-165x165mm-/351457408861?hash=item51d47e775d

ESP8266 Development Board

![NodeMCU](nodemcu.jpg)

http://www.aliexpress.com/item/new-Wireless-module-CH340-NodeMcu-V3-Lua-WIFI-Internet-of-Things-development-board-based-ESP8266/32520574539.html

The ESP8266 comes on a variety of development boards.  See the following YouTube video for some help in choosing which one to use for your project.  For maximum ease of construction, you should choose one that comes with a USB connection & power regulator.

https://www.youtube.com/watch?v=YQadLyBwz3M

To connect up the ESP8266 to the SunAirPlus board, simply connect as follows

|ESP8266       |SunAirPlus|
|--------------|----------|
|3.3V          |VDD|
|GND           |GND|
|D1           |SCL|
|D2           |SDA|

You will also need the following connectors to attach the battery & solar cell to the SunAirPlus board

![JST](jst.jpg)

https://littlebirdelectronics.com.au/products/jst-2-pin-cable

I have now included code to light any LEDs connected from D6,D7,D8 to ground to indicate connection progress, message arrival & (un)successful publication.

|Pin    |Colour |Solid       |Short Flash         |
|-------|-------|------------|--------------------|
|D6     |Red    |WiFi Connecting|Failed to Publish|
|D7     |Yellow |MQTT Connecting|Command Arrived  |
|D8     |Green  |Up & Running |Successful Publish |

