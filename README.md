# ttn-simple-env-monitor

## Info on this repository

For now, Arduino sketches of steps to building a simple TTN v3 LoRaWAN env monitor (temp, humidity, barometric pressure).  This is a work in progress, will not make releases until I have a working  soup-to-nuts version with TTN server forwarding to some sort of backend (which one will decide later).  

At this time,  still leaving in *some* cruft (excessive Serial.print(), discarded or debug code commented out, etc.).  Will make cleanup pass before first "release".

### Apps

* **BasicLoraTestXcvr** - Not actually in this repository, but in another of my public repos 
  (https://github.com/saelkind/lora-test.git).  Takes the "standard" LoRa test transmitter and 
  receiver apps, combining and enhancing to make transceivers sending packets to each other without
  LoRaWAN.  Serves as a check for LoRa dev boards to demo that they're working, both transmit and
  receive, and maybe to test antennas, etc. More info is on the other repository.
  
* **ttn-otaa-hello** - *Hello, World* example, including payload decoder to cut & paste into TTN console.  Arduino sketch, only lightly modified example from MCCI aruduino_lmic repository example.
  * Current target: LilyGO TTGO ESP32 LoRa 915 MHz with OLED display.  Only tested there so far.
  * **TODO** - Port to Feather M0 & test.
  * **TODO** - (Longer term if ever) See if I can port to Lora32u4 and test.  This should fit
    within the FLASH size limit, but I think I need to add lmic state persistence (using FRAM
	or onboard FLASH?) that I think is required for TTN v3, and maybe having to use ABP in place
	of OTAA.

* **ttn-sht31-nono** - (Quick hit step) Transmit SHT31 periodic stats to TTN server.  Still sending a 
  string (hence "nono").

  Quick hit to a new step, based on *ttn-otaa-hello*.  Takes one reading of temp & humidity during 
  *doSend()*, uses *dtostrf()* and *sprintf()* to print floats to a formatted string (char array),
  in place of the *Hello, world!* message, leverages same payload decoder to view on console.  Also
  adds simple display of the same string on the built-in OLED.  Being impatient, I lowered
  the transmission delay time, so it probably violates the TTN fair use policy.  Either don't 
  leave it on too long, or increase the TX delay to a number allowing no more than 9 packets/hr
  based on the payload size (see https://avbentem.github.io/airtime-calculator/ttn/us915/30).
  
  This adds the data conversion to formatted string within the limits of the Arduino subset impl
  of *sprintf()*, reading from an SHT31, and simple use of the onboard OLED display.
  
    ```  
	"decoded_payload": {
          "message": "T:  25.4, RH:  61.6"
     },
	```
  
  * **TODO** - Clean up code.

* **ttn-sht31** - Transmit SHT31 periodic stats to TTN server.  

  Move temperature and humidity sampling to main loop(), then send aggregate stats during *doSend()*.
  Stats to be sent are (min, max, last) for each transmission cycle for &deg;C and RH%.  Do my own
  data representation, not leveraging any framework - send each datum as an int after scaling to
  retain significant digits.
  
  In addition to doing the data conversion and new payload formatter on the TTN side
  
  * **TODO** TBD if will do deeper research on sensor calibration, when and how to use heater, etc.
  (maybe later release?).  I've been observing variations in readings between sensors, 
  so figuring out how to calibrate without expensive equipment will be a must.  Consistent
  values from each sensor is not the issue, it's just the difference between sensors -
  especially between SHT31 and BME280 - is more than the sum of the spec'd tolerances.  I don't
  know if it's the generic breakout boards, or if there's something going on that I should expect
  expect even from those of known quality.
  
* **ttn-simple-env-monitor** - *(not started yet)* Transmit SHT31 and BME280 periodic stats to TTN server.  

  Plan is (min, max, last, flag) for each period for &deg;C, RH%, and barometic pressure (hPa).  
  BME280 temp & humidity will be used solely as sanity check on SHT31 temp & humidity.  Flag field
  will flag if the delta(s) exceed some threshold - or may replace with a pair of period max observed delta fields.


### Dependencies

* MCCI arduino-lmic library (https://github.com/mcci-catena/arduino-lmic)
* Rob Tillart SHT31 library (through Arduino IDE, but also https://github.com/RobTillaart/SHT31)
* *TBD BME280 library* (for pressure, and sanity check on SHT31 readings)
* Espressif ESP32 board library (through Arduino IDE boards manager)
  * I've set the board to "TTGO LoRa32-OLED v1"
* ThingPulse OLED SSD1306 ESP32 library (https://github.com/ThingPulse/esp8266-oled-ssd1306)

### Looking further out
Longer-range plan - simple env monitors in multiple places in my apartment - maybe axe the barometric pressure in most as unnecessary, or instead axe the SHT31 and use only BME280 if I can get consistent calibration with some reference.  Possibly in place of BME280 in at least one monitor, use BME680 to add IAQ estimate.  One indoor monitor will add CO<sub>2</sub> using IR or other single-gas sensor.  The outdoor weather station monitor will eventually add particle counts and maybe CO<sub>2</sub> to the SHT31/BME280 pair.
