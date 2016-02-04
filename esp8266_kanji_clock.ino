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

unsigned int hour = 0;
unsigned int minute = 0;

//-----------
// The 8x8 LED Matrices
//-----------
Adafruit_8x8matrix matrix[4] = {
  Adafruit_8x8matrix() ,
  Adafruit_8x8matrix() ,
  Adafruit_8x8matrix() ,
  Adafruit_8x8matrix() ,  
};
unsigned int digit[4] = {0, 0, 0, 0};

//-----------
// Local network setup
//-----------
const char* ssid     = "XXXX";
const char* password = "XXXX";
unsigned int localPort = 6677;

//-----------
// NTP server setup
// see list here: http://tf.nist.gov/tf-cgi/servers.cgi
// nist-time-server.eoni.com  216.228.192.69  La Grande, Oregon
//-----------
IPAddress NTPServer (216, 228, 192, 69);
const int NTP_PACKET_SIZE = 48;
byte packetBuffer[NTP_PACKET_SIZE];
WiFiUDP udp;
unsigned long sendNTPRequest(); 

//-----------
// bitmaps
//-----------
static PROGMEM const uint8_t KANJI_BLANK[] = 
  { B00000000,
    B00000000,
    B00000000,
    B00000000,
    B00000000,
    B00000000,
    B00000000,
    B00000000  };       
static PROGMEM const uint8_t KANJI_DIGIT[][8] = {
  //   ZERO
  { B00111100,
    B01000010,
    B10000001,
    B10000001,
    B10000001,
    B10000001,
    B01000010,
    B00111100  }  ,
  //   ONE
  { B00000000,
    B00000000,
    B00000000,
    B00000000,
    B11111111,
    B00000000,
    B00000000,
    B00000000  }  ,
  //   TWO
  { B00000000,
    B00000000,
    B01111110,
    B00000000,
    B00000000,
    B00000000,
    B11111111,
    B00000000  }  ,
  //   THREE
  { B00000000,
    B01111110,
    B00000000,
    B00000000,
    B00111100,
    B00000000,
    B00000000,
    B11111111  }  ,
  //   FOUR
  { B11111111,
    B10101001,
    B10101001,
    B10101011,
    B11001101,
    B10000001,
    B11111111,
    B10000001  }  ,
  //   FIVE
  { B00000000,
    B01111110,
    B00010000,
    B00010000,
    B01111110,
    B00010010,
    B00100010,
    B11111111  }  ,
  //   SIX
  { B00100000,
    B00010000,
    B00010000,
    B11111111,
    B00000000,
    B00100100,
    B01000010,
    B10000001  }  ,
  //   SEVEN
  { B00000000,
    B00010000,
    B00010000,
    B01111110,
    B00010000,
    B00010000,
    B00010001,
    B00011110  }  ,
  //   EIGHT
  { B00000000,
    B00101000,
    B00101000,
    B00101000,
    B00101000,
    B00100100,
    B01000110,
    B10000011  }  ,
  //   NINE
  { B00100000,
    B00100000,
    B11111100,
    B00100100,
    B00100100,
    B00100100,
    B01000101,
    B10000110  }  ,
};

//--------------------------------------------------------
// byteFlip
// "00110011" becomes "11001100"
//
// found here:
//    http://www.nrtm.org/index.php/2013/07/25/reverse-bits-in-a-byte/
//--------------------------------------------------------
byte byteFlip(byte num) {
  byte var = 0;     
  int i, x, y, p;
  int s = 8;    // number of bits in 'num'.
 
  for (i = 0; i < (s / 2); i++) {
    // extract bit on the left, from MSB
    p = s - i - 1;
    x = num & (1 << p);
    x = x >> p;  
    // extract bit on the right, from LSB
    y = num & (1 << i);
    y = y >> i;
 
    var = var | (x << i);       // apply x
    var = var | (y << p);       // apply y
  }
  return var;
}
  
//--------------------------------------------------------
// scroll_down_bmp
//
// replace current contents with bitmap via scroll
//--------------------------------------------------------
void scroll_down_bmp(const uint8_t *bitmap, Adafruit_8x8matrix* matrix) {
  uint8_t row;
  for (int i=0; i<=7; i++) {
    for (int y=7; y>=0; y--) {
      if (y>i) {
        matrix->displaybuffer[y] = matrix->displaybuffer[y-1];
      } else {
        row = pgm_read_byte(bitmap + (y+7-i));    // read the byte from PROGMEM
        row = byteFlip(row);                      // fixes apparent LSB/MSB disagreement
        row = row << 7 | row >> 1;                // rotate to fix memory buffer error
        matrix->displaybuffer[y] = row;           // set it
      }
    }
    matrix->writeDisplay();
    delay(150);
  }
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

  for (int i=0; i<4; i++) {
    if (D[i]!=digit[i]) {
      digit[i] = D[i];
      scroll_down_bmp(KANJI_DIGIT[digit[i]], &matrix[i]); 
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
    scroll_down_bmp(KANJI_DIGIT[digit[i]], &matrix[i]);
  }  

  // Connect to WiFi network
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
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
  udp.begin(localPort);
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
