# ESP8266_Kanji_Clock
![thumbnail](http://caternuson.github.io/kanji-clock-thumb.jpg)<br/>
Arduino code for ESP8266 based kanji clock.

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
Create a file called ```network_config.h``` and define your network configuration:
```c++
#define MY_SSID "your_ssid"
#define MY_PASSWORD "your_password"
```
Change the NTP address to nearest ![server](http://tf.nist.gov/tf-cgi/servers.cgi):
```c++
IPAddress NTPServer (216, 228, 192, 69);
```
TODO: add configurable GMT offset

# Build
Open ```esp8266_kanji_clock.ino``` in the Arduino IDE and load onto ESP8266.

# NTP
Info on NTP.
