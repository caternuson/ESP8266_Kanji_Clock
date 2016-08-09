//================================================================
// esp8266_kanji_clock.ino
//
// An ESP8266 based NTP clock which displays time using kanji
// characters on 8x8 LED matrix displays.
//  * Adafruit ESP8266 Huzzah
//  * Adafruit 8x8 LED Matrix (4)
//  * wood, glass, and other stuff
//
// Carter Nelson
// 2016-02-02
//================================================================
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <Adafruit_LEDBackpack.h>

#include "kanji_clock_config.h"    

unsigned int hour = 0;
unsigned int minute = 0;
unsigned int digit[4] = {0, 0, 0, 0};

//-----------
// The 8x8 LED Matrices
//-----------
Adafruit_8x8matrix matrix[4] = {
  Adafruit_8x8matrix() ,
  Adafruit_8x8matrix() ,
  Adafruit_8x8matrix() ,
  Adafruit_8x8matrix() ,  
};

//-----------
// NTP stuff
//-----------
#define NTP_PACKET_SIZE 48
byte packetBuffer[NTP_PACKET_SIZE];
WiFiUDP udp;

//-----------
// Bitmaps
//-----------
static const uint64_t KANJI_BLANK = 0x0000000000000000;
static const uint64_t KANJI_DIGIT[] =
  {
    0x3c4281818181423c ,    // 0
    0x000000ff00000000 ,    // 1
    0x00ff0000007e0000 ,    // 2     
    0xff00003c00007e00 ,    // 3
    0x81ff81b3d59595ff ,    // 4
    0xff44487e08087e00 ,    // 5
    0x81422400ff080804 ,    // 6
    0x788808087e080800 ,    // 7
    0xc162241414141400 ,    // 8
    0x61a22424243f0404      // 9   
  };

//--------------------------------------------------------
// scroll_raw64
//
// Scroll out the current bitmap with the supplied bitmap.
// Can also specify a matrix (0-3) and a delay to set scroll
// rate.
//--------------------------------------------------------
void scroll_raw64(uint64_t value, uint8_t m, uint8_t d) {
  uint8_t row_byte;
  for (int s=7; s>=0; s--) {
    for (int r=7; r>=1; r--) {
      matrix[m].displaybuffer[r] = matrix[m].displaybuffer[r-1];
    }
    row_byte = (value >> (8*s)) & 0xff;
    row_byte = (row_byte << 7 | row_byte >> 1) & 0xff;
    matrix[m].displaybuffer[0] = row_byte;
    matrix[m].writeDisplay();
    delay(d);
  }
}

//--------------------------------------------------------
// set_raw64
//
// Set specified matrix to bitmap defined by 64 bit value.
//--------------------------------------------------------
void set_raw64(uint64_t value, uint8_t m) {
  uint8_t row_byte, pixel_bit;
  for (int y=0; y<=7; y++) {
    row_byte = value >> (8*y);
    for (int x=0; x<=7; x++) {
      pixel_bit = row_byte >> x & 0x01;
      matrix[m].drawPixel(x, y, pixel_bit);
    }
  }
  matrix[m].writeDisplay();
}

//--------------------------------------------------------
// is_daylight_saving
//
// Return true if Daylight Saving is ON
//--------------------------------------------------------
bool is_daylight_saving() {
  return digitalRead(12);
}

//--------------------------------------------------------
// get_NTP_time
//
// Send request to NTP server. See RFC 5905 for UDP
// datagram description:
//   https://tools.ietf.org/html/rfc5905
//--------------------------------------------------------
void get_NTP_time() {
  // Set up buffer for request
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  packetBuffer[0] = 0xE3;

  // Make request
  udp.beginPacket("pool.ntp.org", 123);
  udp.write(packetBuffer, NTP_PACKET_SIZE);
  udp.endPacket();

  // Process request
  int bytes_recvd = udp.parsePacket();
  if (!bytes_recvd) {
    Serial.println("Nothing received.");
  } else {
    Serial.print("Received packet length=");
    Serial.println(bytes_recvd);

    // Read response into buffer
    udp.read(packetBuffer, NTP_PACKET_SIZE);

    // Extract seconds since 1900
    uint32_t time_1900 = packetBuffer[40] << 24 | 
                         packetBuffer[41] << 16 |
                         packetBuffer[42] <<  8 |
                         packetBuffer[43] ;

    // Convert to local time
    uint32_t time_local = time_1900 + (GMT_OFFSET * 3600);

    // Adjust for Daylight Saving
    if (is_daylight_saving()) time_local += 3600;   

    // Update globals
    hour = int((time_local % 86400L) / 3600);
    minute = int((time_local % 3600) / 60);    
  }
}

//--------------------------------------------------------
// display_time
//
// display the current time on the 8x8 LED matrices
//--------------------------------------------------------
void display_time() {
  // Compute digits
  unsigned int D[4] = {
    hour / 10,
    hour % 10,
    minute / 10,
    minute % 10
  };

  Serial.print("DISPLAY: ");
  Serial.print("[");
  Serial.print(D[0]);
  Serial.print("][");
  Serial.print(D[1]);
  Serial.print("][");
  Serial.print(D[2]);
  Serial.print("][");
  Serial.print(D[3]);
  Serial.println("]");

  // Update digits that have changed
  for (int i=0; i<4; i++) {
    if (D[i]!=digit[i]) {
      digit[i] = D[i];
      scroll_raw64(KANJI_DIGIT[digit[i]], i, SCROLL_DELAY);
    }
  } 
}

//--------------------------------------------------------
//                S  E  T  U  P
//--------------------------------------------------------
void setup() {
  Serial.begin(115200);
  Serial.println("ESP8266 NTP Kanji Clock");

  // Pins used for daylight saving switch
  pinMode(12, INPUT_PULLUP);
  pinMode(14, OUTPUT);
  digitalWrite(14, LOW);
  pinMode(16, INPUT_PULLUP);

  // Initialize 8x8 LED matrices
  matrix[0].begin(0x70);  // tens digit for hour
  matrix[1].begin(0x71);  // ones digit for hour
  matrix[2].begin(0x72);  // tens digit for minute
  matrix[3].begin(0x73);  // ones digit for minute
  for (int i=0; i<4; i++) {
    set_raw64(KANJI_DIGIT[digit[i]], i);
  }  

  // Connect to WiFi network
  Serial.print("Connecting to ");
  Serial.println(MY_SSID);
  WiFi.begin(MY_SSID, MY_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("WiFi connected. [ip addr ");  
  Serial.print(WiFi.localIP());
  Serial.println("]");

  // Start UDP
  Serial.print("Starting UDP...");
  udp.begin(PORT);
  Serial.print("Done. [port ");
  Serial.print(udp.localPort());
  Serial.println("]"); 
}

//--------------------------------------------------------
//                    L  O  O  P
//--------------------------------------------------------
void loop() {
  get_NTP_time();     // get the time
  display_time();     // display it
  delay(10000);       // wait 10 seconds
}
