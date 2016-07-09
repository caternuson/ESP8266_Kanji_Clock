# ESP8266 Kanji Clock
![thumbnail](http://caternuson.github.io/kanji-clock-thumb.jpg)<br/>
A clock that display time using kanji digits.

# Hardware
* Adafruit ![HUZZAH ESP8266 Breakout](https://www.adafruit.com/products/2471)
* Adafruit ![Mini 8x8 LED Matrix w/I2C Backpack](https://www.adafruit.com/products/870)
    * QUA = 4, any color
* USB console cable for programming
    
# Software
The main program is ```esp8266_kanji_clock.ino```.

# Dependencies
* ESP8266 Board Package for the Arduino IDE
    * Follow ![these](https://learn.adafruit.com/adafruit-huzzah-esp8266-breakout/using-arduino-ide) instructions.
* Adafruit LED Backpack library
* Adafruit GFX library

# Install
Clone this repo:
```
$ git clone https://github.com/caternuson/ESP8266_Kanji_Clock.git
```

# Configure
Create a file called ```kanji_clock_config.h``` with the following contents:
```c++
// Local network configuration
#define MY_SSID "yourssid"
#define MY_PASSWORD "yourpassword"
#define PORT 6677

// Local GMT offset
#define GMT_OFFSET 0

// Scroll delay in millisecs
#define SCROLL_DELAY 150
```
Change the Wifi network ID and password accordingly. Also set the ```GMT_OFFSET```
to you local time zone. The display scroll speed can be controlled by altering
the ```SCROLL_DELAY```.

# Build
Open ```esp8266_kanji_clock.ino``` in the Arduino IDE and load onto ESP8266.

# NTP
Info on NTP.
