//================================================================
// esp8266_kanji_clock.ino
//
// An ESP8266 based NTP clock which displays time using kanji
// characters on 8x8 LED matrix displays.
//  * Adafruit ESP8266 Huzzah
//  * Adafruit 8x8 LED Matrix (4)
//  * other stuff
//
// Carter Nelson
// 2016-02-02
//================================================================
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <Adafruit_LEDBackpack.h>
#include <Adafruit_GFX.h>

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
// Local network setup
//-----------
//unsigned int localPort = PORT;

//-----------
// NTP server setup
// see list here: http://tf.nist.gov/tf-cgi/servers.cgi
// nist-time-server.eoni.com  216.228.192.69  La Grande, Oregon
//-----------
IPAddress NTPServer NTP_SERVER_ADDR;
byte packetBuffer[NTP_PACKET_SIZE];
WiFiUDP udp;
//unsigned long sendNTPRequest(); 

//-----------
// bitmaps
//-----------
static const uint64_t KANJI_BLANK = 0x0000000000000000;
static const uint64_t KANJI_DIGIT[] =
  {
    0x3c4281818181423c ,
    0x000000ff00000000 ,
    0x00ff0000007e0000 ,
    0xff00003c00007e00 ,
    0x81ff81b3d59595ff ,
    0xff44487e08087e00 ,
    0x81422400ff080804 ,
    0x788808087e080800 ,
    0xc162241414141400 ,
    0x61a22424243f0404   
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
// get_NTP_time
//
// Send request to NTP server. Reused NTP related code
// found here:
//   https://github.com/sandeepmistry/esp8266-Arduino/blob/master/esp8266com/esp8266/libraries/ESP8266WiFi/examples/NTPClient/NTPClient.ino
// See RFC 5905 for UDP datagram description:
//   https://tools.ietf.org/html/rfc5905
//--------------------------------------------------------
void get_NTP_time() {
  Serial.println("sending NTP packet...");
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;     // LI, Version, Mode
  packetBuffer[1] = 0;              // Stratum, or type of clock
  packetBuffer[2] = 6;              // Polling Interval
  packetBuffer[3] = 0xEC;           // Peer Clock Precision 0xEC = -20
  // [4]-[7]  = 0                   // Root Delay
  // [8]-[11] = 0                   // Root Dispersion
  packetBuffer[12]  = 49;           // '1' four-character ASCII "kiss code"
  packetBuffer[13]  = 0x4E;         // 'N'           "
  packetBuffer[14]  = 49;           // '1'           "
  packetBuffer[15]  = 52;           // '4'           "

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  udp.beginPacket(NTPServer, 123); //NTP requests are to port 123
  udp.write(packetBuffer, NTP_PACKET_SIZE);
  udp.endPacket();

  // deal with UDP packet
  int cb = udp.parsePacket();
  if (!cb) {
    Serial.println("no packet yet");
  }
  else {
    Serial.print("packet received, length=");
    Serial.println(cb);
    // We've received a packet, read the data from it
    udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer

    //the timestamp starts at byte 40 of the received packet and is four bytes,
    // or two words, long. First, esxtract the two words:

    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
    // combine the four bytes (two words) into a long integer
    // this is NTP time (seconds since Jan 1 1900):
    unsigned long secsSince1900 = highWord << 16 | lowWord;
    //Serial.print("Seconds since Jan 1 1900 = " );
    //Serial.println(secsSince1900);

    // now convert NTP time into everyday time:
    // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
    const unsigned long seventyYears = 2208988800UL;
    // subtract seventy years:
    unsigned long epoch = secsSince1900 - seventyYears;
    // print Unix time:
    //Serial.print("Unix time = ");
    //Serial.println(epoch);

    // adjust epoch to local time
    // UTC is the time at Greenwich Meridian (GMT)
    // Seattle = UTC - 8 hours
    unsigned long epoch_SEA = epoch - (8 * 3600);
    
    // set globals used by clock display
    hour = int((epoch_SEA  % 86400L) / 3600);
    minute = int((epoch_SEA % 3600) / 60);

    // print local time
    //Serial.print("Seattle time is ");
    //Serial.print(hour);
    //Serial.print(":");
    //Serial.println(minute);
  }
}

//--------------------------------------------------------
// display_time
//
// display the current time on the 8x8 LED matrices
//--------------------------------------------------------
void display_time() {
  
  // compute digits
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

  // update digits that have changed
  for (int i=0; i<4; i++) {
    if (D[i]!=digit[i]) {
      digit[i] = D[i];
      scroll_raw64(KANJI_DIGIT[digit[i]], i, SCROLL_DELAY);
    }
  } 
}

//--------------------------------------------------------
// S E T U P
//--------------------------------------------------------
void setup() {
  Serial.begin(115200);
  Serial.println("ESP8266 NTP Kanji Clock");

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
// L O O P
//--------------------------------------------------------
void loop() {
  get_NTP_time();     // get the time
  display_time();     // display it
  delay(10000);       // wait 10 seconds
}
