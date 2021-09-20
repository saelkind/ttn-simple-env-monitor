# ttn-simple-env-monitor

## Info on this repository

For now, Arduino sketches of steps to building simple TTN v3 LoRaWAN env monitor (temp, humidity, 
barometric pressure).  No releases until I have a working version soup-to-nuts with TTN server
forwarding to some sort of backend (which one will decide later).  At this time, 
still leaving in *some* cruft (excessive Serial.print(), discarded or debug code commented out, etc.).  
Will make cleanup pass before "release".

### Apps

* **BasicLoraTestXcvr** - Not actually in this repository, but in another of my public repos 
 (https://github.com/saelkind/lora-test.git).  Takes the "standard" LoRa test transmitter and 
 receiver apps, combining and enhancing to make transceivers sending packets to each other.  Serves as
 check for LoRa dev boards to demo that they're working, and maybe to test antennas, etc. More
 info there.

* **ttn-otaa-hello** - *Hello, World* example, including payload decoder to cut & paste into TTN console.  
Arduino sketch only lightly modified example from MCCI aruduino_lmic repository example.
  * Current target: LilyGO TTGO ESP32 LoRa 915 MHz with OLED display.  Tested there only so far
  * TODO - Port to Feather M0 & test.
  * TODO - Enable display if present, write packet count, status, RSSI, ... on it.
  * TODO - (unlikely) Figure out ESP32 low power mode so can operate for x days on battery without recharge

* **ttn-sht31** - *(not started yet)* Transmit SHT31 periodic stats to TTN server.  Plan is
  (min, max, avg) for each period for &deg;C and RH%.  TBD if will do deeper research on 
  sensor calibration, when and how to use heater, etc. (maybe later release?)
  
* **ttn-simple-env-monitor** - *(not started yet)* Transmit SHT31 and BME280 periodic stats to TTN server.  
 Plan is (min, max, avg, flag) for each period for &deg;C, RH%, and barometic pressure (hPA).  BME280 temp &
 humidity will be used solely as sanity check on SHT31 temp & humidity.  Flag field will flag if the delta(s)
 exceed some threshold - or may replace with a pair of maxDelta fields.


### Dependencies

* MCCI arduino-lmic library (https://github.com/mcci-catena/arduino-lmic)
* Rob Tillart SHT31 library (through Arduino IDE, but also https://github.com/RobTillaart/SHT31)
* *TBD BME280 library* (for pressure, and sanity check on SHT31 readings)
* Espressif ESP32 board library (through Arduion IDE boards manager)
  * I've set the board to "TTGO LoRa32-OLED v1"
* ThingPulse OLED SSD1306 ESP32 library (https://github.com/ThingPulse/esp8266-oled-ssd1306)
