# ESP8266 Kanji Clock
![thumbnail](http://caternuson.github.io/kanji-clock-thumb.jpg)<br/>
Displays the current time using kanji digits.

# Hardware
* Adafruit ![HUZZAH ESP8266 Breakout](https://www.adafruit.com/products/2471)
* Adafruit ![Mini 8x8 LED Matrix w/I2C Backpack](https://www.adafruit.com/products/870)
    * any color x 4
* Wood and glass
* ![USB Console Cable](https://www.adafruit.com/products/954) for programming
    
# Software
The main program is ```esp8266_kanji_clock.ino```.

# Dependencies
* ESP8266 Board Package for the Arduino IDE
    * Follow ![these](https://learn.adafruit.com/adafruit-huzzah-esp8266-breakout/using-arduino-ide) instructions.
* ![Adafruit LED Backpack library](https://github.com/adafruit/Adafruit_LED_Backpack)

# Configure (REQUIRED)
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
Change the Wifi network ID and password accordingly. Also set the `GMT_OFFSET`
to you local time zone. The display scroll speed can be controlled by altering
the `SCROLL_DELAY`.

# Install
Clone this repo:
```
$ git clone https://github.com/caternuson/ESP8266_Kanji_Clock.git
```
and then use the Arduino IDE with ESP8266 board package to upload the sketch.
Details on how to programm the HUZZAH can be found
![here](https://learn.adafruit.com/adafruit-huzzah-esp8266-breakout).

# Icon Summary
| ICON | VALUE |
|:---:|:---:|
|![1](http://caternuson.github.io/ESP8266_Kanji_Clock/static/kanji_1.jpg)| 1 |
|![2](http://caternuson.github.io/ESP8266_Kanji_Clock/static/kanji_2.jpg)| 2 |
|![3](http://caternuson.github.io/ESP8266_Kanji_Clock/static/kanji_3.jpg)| 3 |
|![4](http://caternuson.github.io/ESP8266_Kanji_Clock/static/kanji_4.jpg)| 4 |
|![5](http://caternuson.github.io/ESP8266_Kanji_Clock/static/kanji_5.jpg)| 5 |
|![6](http://caternuson.github.io/ESP8266_Kanji_Clock/static/kanji_6.jpg)| 6 |
|![7](http://caternuson.github.io/ESP8266_Kanji_Clock/static/kanji_7.jpg)| 7 |
|![8](http://caternuson.github.io/ESP8266_Kanji_Clock/static/kanji_8.jpg)| 8 |
|![9](http://caternuson.github.io/ESP8266_Kanji_Clock/static/kanji_9.jpg)| 9 |
|![0](http://caternuson.github.io/ESP8266_Kanji_Clock/static/kanji_0.jpg)| 0 |
