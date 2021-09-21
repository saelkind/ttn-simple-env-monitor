# ttn-simple-env-monitor

## Info on this repository

For now, Arduino sketches of steps to building a simple TTN v3 LoRaWAN env monitor (temp, humidity, barometric pressure).  This is a work in progress, will not make releases until I have a working  soup-to-nuts version with TTN server forwarding to some sort of backend (which one will decide later).  

At this time,  still leaving in *some* cruft (excessive Serial.print(), discarded or debug code commented out, etc.).  Will make cleanup pass before first "release".

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

* **ttn-sht31** - *(not started yet)* Transmit SHT31 periodic stats to TTN server.  

  Plan is (min, max, avg) for each period for &deg;C and RH%.  TBD if will do deeper research on 
  sensor calibration, when and how to use heater, etc. (maybe later release?).  I've been observing variations 
  in readings between sensors, in excess of spec'd accuracy - especially between SHT31 and BME280 - so
  figuring out how to calibrate without expensive equipment will be a must.
  
* **ttn-simple-env-monitor** - *(not started yet)* Transmit SHT31 and BME280 periodic stats to TTN server.  

  Plan is (min, max, avg, flag) for each period for &deg;C, RH%, and barometic pressure (hPa).  BME280 temp &
  humidity will be used solely as sanity check on SHT31 temp & humidity.  Flag field will flag if the delta(s)
  exceed some threshold - or may replace with a pair of period max observed delta fields.


### Dependencies

* MCCI arduino-lmic library (https://github.com/mcci-catena/arduino-lmic)
* Rob Tillart SHT31 library (through Arduino IDE, but also https://github.com/RobTillaart/SHT31)
* *TBD BME280 library* (for pressure, and sanity check on SHT31 readings)
* Espressif ESP32 board library (through Arduino IDE boards manager)
  * I've set the board to "TTGO LoRa32-OLED v1"
* ThingPulse OLED SSD1306 ESP32 library (https://github.com/ThingPulse/esp8266-oled-ssd1306)

### Looking further out
Longer-range plan - simple env monitors in multiple places in my apartment (maybe axe the barometric pressure in most, or instead axe the SHT31 and use only BME280.  Possibly in place of BME280 in at least one BME680 to add IAQ estimate.  One indoor monitor will add CO<sub>2</sub> using IR or other single-gas sensor.  The outdoor weather station monitor will eventually add particle counts and maybe CO<sub>2</sub> to the SHT31/BME280 pair.
