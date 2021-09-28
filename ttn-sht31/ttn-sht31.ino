/*******************************************************************************
 * Copyright (c) 2015 Thomas Telkamp and Matthijs Kooijman
 * Copyright (c) 2018 Terry Moore, MCCI
 *
 * Permission is hereby granted, free of charge, to anyone
 * obtaining a copy of this document and accompanying files,
 * to do whatever they want with them without any restriction,
 * including, but not limited to, copying, modification and redistribution.
 * NO WARRANTY OF ANY KIND IS PROVIDED.
 *
 * This example sends a valid LoRaWAN packet with payload "Hello,
 * world!", using frequency and encryption settings matching those of
 * the The Things Network.
 *
 * This uses OTAA (Over-the-air activation), where where a DevEUI and
 * application key is configured, which are used in an over-the-air
 * activation procedure where a DevAddr and session keys are
 * assigned/generated for use with all further communication.
 *
 * Note: LoRaWAN per sub-band duty-cycle limitation is enforced (1% in
 * g1, 0.1% in g2), but not the TTN fair usage policy (which is probably
 * violated by this sketch when left running for longer)!

 * To use this sketch, first register your application and device with
 * the things network, to set or generate an AppEUI, DevEUI and AppKey.
 * Multiple devices can use the same AppEUI, but each device has its own
 * DevEUI and AppKey.
 *
 * Do not forget to define the radio type correctly in
 * arduino-lmic/project_config/lmic_project_config.h or from your BOARDS.txt.
 *
 *******************************************************************************/

#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>
#include <Wire.h>
#include <SSD1306.h>
#include <SHT31.h>

//
// For normal use, we require that you edit the sketch to replace FILLMEIN
// with values assigned by the TTN console. However, for regression tests,
// we want to be able to compile these scripts. The regression tests define
// COMPILE_REGRESSION_TEST, and in that case we define FILLMEIN to a non-
// working but innocuous value.
//
#ifdef COMPILE_REGRESSION_TEST
# define FILLMEIN 0
#else
# warning "You must replace the values marked FILLMEIN with real values from the TTN control panel!"
# define FILLMEIN (#dont edit this, edit the lines that use FILLMEIN)
#endif

#define OLED_SDA    4
#define OLED_SCL    15
#define OLED_RST    16

#define SHT31_ADDRESS   0x44
SHT31 sht;

struct EnvDataFloat {
  float maxTemp;
  float minTemp;
  float cumTemp;
  float tempSamps;
  float lastTemp;
  float maxRhPerc;
  float minRhPerc;
  float cumRhPerc;
  float rhPercSamps;
  float lastRhPerc;
  float maxPressHPa;
  float minPressHPa;
  float cumPressHPa;
  float pressHPaSamps;  
  float lastPressHPa;
};

struct EnvDataInt {
  int avgTemp;
  int lastTemp;
  int avgRhPerc;
  int lastRhPerc;
  int avgPressHPa;
  int lastPressHPa;
};

union EnvDataPacket {
  struct EnvDataInt envData;
  uint8_t uints[sizeof(EnvDataInt)];
};

// This EUI must be in little-endian format, so least-significant-byte
// first. When copying an EUI from ttnctl output, this means to reverse
// the bytes. For TTN issued EUIs the last bytes should be 0xD5, 0xB3,
// 0x70.
static const u1_t PROGMEM APPEUI[8]={ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  };  // was fillmein
void os_getArtEui (u1_t* buf) { memcpy_P(buf, APPEUI, 8);}

// This should also be in little endian format, see above.
static const u1_t PROGMEM DEVEUI[8]={ 0x2E, 0x52, 0x04, 0xD0, 0x7E, 0xD5, 0xB3, 0x70 };   // was fillmein
void os_getDevEui (u1_t* buf) { memcpy_P(buf, DEVEUI, 8);}

// This key should be in big endian format (or, since it is not really a
// number but a block of memory, endianness does not really apply). In
// practice, a key taken from ttnctl can be copied as-is.
static const u1_t PROGMEM APPKEY[16] = { 0xC8, 0x2F, 0x21, 0x8B, 0x27, 0x59, 0x17, 0x40, 0xBC, 0x66, 0x2E, 0x1B, 0xA0, 0x58, 0x8F, 0x85 };    // was fillmein
void os_getDevKey (u1_t* buf) {  memcpy_P(buf, APPKEY, 16);}

//static uint8_t mydata[] = "Hello, world!";
static osjob_t sendjob;

SSD1306 display(0x3c, OLED_SDA, OLED_SCL);

struct EnvSample {
  float tempC;
  float rhPerc;
};
static struct EnvSample envSample;

union msg {
   uint8_t uints[40];
   char envDataStr[40];
};
static union msg message;

// Schedule TX every this many seconds (might become longer due to duty
// cycle limitations).
const unsigned TX_INTERVAL = 180;

/*static char PROGMEM EV_STR_NONE[] = "No event";
static char PROGMEM EV_STR_OTHER[] = "Other";
static char PROGMEM EV_STR_UNKNOWN[] = "Unknown";
static char PROGMEM EV_STR_JOINING[] = "Joining";
static char PROGMEM EV_STR_JOINED[] = "Joined";
static char PROGMEM EV_STR_JOIN_FAILED[] = "Join Fail";
static char PROGMEM EV_STR_REJOIN_FAILED[] = "Rejoin Fail";
static char PROGMEM EV_STR_TXCOMPLETE[] = "TX Complete";
static char PROGMEM EV_STR_LOST_TSYNC[] = "Lost Tsync";
static char PROGMEM EV_STR_RESET[] = "Reset";
static char PROGMEM EV_STR_RXCOMPLETE[] = "RX Complete";
static char PROGMEM EV_STR_TXCANCELED[] = "TX Start";
static char PROGMEM EV_STR_RXSTART[] = "RX Start";
static char PROGMEM EV_STR_TXSTART[] = "TX Start";
static char PROGMEM EV_STR_JOIN_TXCOMPLETE[] = "Join TX Complete";
static char* EV_STR_SCAN_TIMEOUT =  EV_STR_OTHER;
static char* EV_STR_BEACON_FOUND = EV_STR_OTHER;
static char* EV_STR_BEACON_MISSED =  EV_STR_OTHER;
static char* EV_STR_BEACON_TRACKED =  EV_STR_OTHER;
static char* EV_STR_LINK_DEAD =  EV_STR_OTHER;
static char* EV_STR_LINK_ALIVE =  EV_STR_OTHER;
//static char PROGMEM EV_STR_SCAN_TIMEOUT[] = "Scan Timeout";
//static char PROGMEM EV_STR_BEACON_FOUND[] = "Bcn Found";
//static char PROGMEM EV_STR_BEACON_MISSED[] = "Bcn Missed";
//static char PROGMEM EV_STR_BEACON_TRACKED[] = "Bcn Tracked";
//static char PROGMEM EV_STR_LINK_DEAD[] = "Link Dead";
//static char PROGMEM EV_STR_LINK_ALIVE[] = "Link Alive";
char* lastEventStr = EV_STR_NONE;*/

// Pin mapping
// redone for TTGO ESP32 LoRa 915  MHz v1.0
const lmic_pinmap lmic_pins = {
    .nss = 18,                       // chip select on feather (rf95module) CS
    .rxtx = LMIC_UNUSED_PIN,
    .rst = 14,                       // reset pin
    .dio = {26, 33, 32}, // no external jumpers needed in v1.0 TTGO (DIO0, DIO1, DIO2)
};

void printHex2(unsigned v) {
    v &= 0xff;
    if (v < 16)
        Serial.print('0');
    Serial.print(v, HEX);
}


void formPacketBody() {
    char tempStr[10];
    dtostrf(envSample.tempC, 5, 1, tempStr);
    char rhStr[10];
    dtostrf(envSample.rhPerc, 5, 1, rhStr);
    sprintf(message.envDataStr, "T: %s, RH: %s", tempStr, rhStr);
}

void updateDisplay() {
    display.clear();
    display.setFont(ArialMT_Plain_10);
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.drawString(display.getWidth() / 2, 20, message.envDataStr);
    /*display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.drawString(4, 46, F("Last Event:"));
    display.drawString(58, 46, lastEventStr);*/
    display.display();
}

void onEvent (ev_t ev) {
    Serial.print(os_getTime());
    Serial.print(": ");
    switch(ev) {
        case EV_SCAN_TIMEOUT:
            Serial.println(F("EV_SCAN_TIMEOUT"));
            //lastEventStr = EV_STR_SCAN_TIMEOUT;
            break;
        case EV_BEACON_FOUND:
            Serial.println(F("EV_BEACON_FOUND"));
            //lastEventStr = EV_STR_BEACON_FOUND;
            break;
        case EV_BEACON_MISSED:
            Serial.println(F("EV_BEACON_MISSED"));
            //lastEventStr = EV_STR_BEACON_MISSED;
            break;
        case EV_BEACON_TRACKED:
            Serial.println(F("EV_BEACON_TRACKED"));
            //lastEventStr = EV_STR_BEACON_TRACKED;
            break;
        case EV_JOINING:
            Serial.println(F("EV_JOINING"));
            //lastEventStr = EV_STR_JOINING;
            break;
        case EV_JOINED:
            Serial.println(F("EV_JOINED"));
            //lastEventStr = EV_STR_JOINED;
            {
              u4_t netid = 0;
              devaddr_t devaddr = 0;
              u1_t nwkKey[16];
              u1_t artKey[16];
              LMIC_getSessionKeys(&netid, &devaddr, nwkKey, artKey);
              Serial.print("netid: ");
              Serial.println(netid, DEC);
              Serial.print("devaddr: ");
              Serial.println(devaddr, HEX);
              Serial.print("AppSKey: ");
              for (size_t i=0; i<sizeof(artKey); ++i) {
                if (i != 0)
                  Serial.print("-");
                printHex2(artKey[i]);
              }
              Serial.println("");
              Serial.print("NwkSKey: ");
              for (size_t i=0; i<sizeof(nwkKey); ++i) {
                      if (i != 0)
                              Serial.print("-");
                      printHex2(nwkKey[i]);
              }
              Serial.println();
            }
            // Disable link check validation (automatically enabled
            // during join, but because slow data rates change max TX
	          // size, we don't use it in this example.
            LMIC_setLinkCheckMode(0);
            break;
        /*
        || This event is defined but not used in the code. No
        || point in wasting codespace on it.
        ||
        || case EV_RFU1:
        ||     Serial.println(F("EV_RFU1"));
        ||     break;
        */
        case EV_JOIN_FAILED:
            Serial.println(F("EV_JOIN_FAILED"));
            //lastEventStr = EV_STR_JOIN_FAILED;
            break;
        case EV_REJOIN_FAILED:
            Serial.println(F("EV_REJOIN_FAILED"));
            //lastEventStr = EV_STR_REJOIN_FAILED;
            break;
        case EV_TXCOMPLETE:
            Serial.println(F("EV_TXCOMPLETE (includes waiting for RX windows)"));
            //lastEventStr = EV_STR_TXCOMPLETE;
            if (LMIC.txrxFlags & TXRX_ACK)
              Serial.println(F("Received ack"));
            if (LMIC.dataLen) {
              Serial.print(F("Received "));
              Serial.print(LMIC.dataLen);
              Serial.println(F(" bytes of payload"));
            }
            // Schedule next transmission
            os_setTimedCallback(&sendjob, os_getTime()+sec2osticks(TX_INTERVAL), do_send);
            break;
        case EV_LOST_TSYNC:
            Serial.println(F("EV_LOST_TSYNC"));
            //lastEventStr = EV_STR_LOST_TSYNC;
            break;
        case EV_RESET:
            Serial.println(F("EV_RESET"));
            //lastEventStr = EV_STR_RESET;
            break;
        case EV_RXCOMPLETE:
            // data received in ping slot
            Serial.println(F("EV_RXCOMPLETE"));
            //lastEventStr = EV_STR_RXCOMPLETE;
            break;
        case EV_LINK_DEAD:
            Serial.println(F("EV_LINK_DEAD"));
            //lastEventStr = EV_STR_LINK_DEAD;
            break;
        case EV_LINK_ALIVE:
            Serial.println(F("EV_LINK_ALIVE"));
            //lastEventStr = EV_STR_LINK_ALIVE;
            break;
        /*
        || This event is defined but not used in the code. No
        || point in wasting codespace on it.
        ||
        || case EV_SCAN_FOUND:
        ||    Serial.println(F("EV_SCAN_FOUND"));
        ||    break;
        */
        case EV_TXSTART:
            Serial.println(F("EV_TXSTART"));
            //lastEventStr = EV_STR_TXSTART;
            break;
        case EV_TXCANCELED:
            Serial.println(F("EV_TXCANCELED"));
            //lastEventStr = EV_STR_TXCANCELED;
            break;
        case EV_RXSTART:
            /* do not print anything -- it wrecks timing */
            //lastEventStr = EV_STR_RXSTART;
            break;
        case EV_JOIN_TXCOMPLETE:
            Serial.println(F("EV_JOIN_TXCOMPLETE: no JoinAccept"));
            //lastEventStr = EV_STR_TXCOMPLETE;
            break;

        default:
            Serial.print(F("Unknown event: "));
            Serial.println((unsigned) ev);
            //lastEventStr = EV_STR_UNKNOWN;
            break;
    }
    updateDisplay();
}

void do_send(osjob_t* j){
    // Check if there is not a current TX/RX job running
    if (LMIC.opmode & OP_TXRXPEND) {
        Serial.println(F("OP_TXRXPEND, not sending"));
    } else {
        envSample = readEnvData();
        Serial.print(envSample.tempC, 2);
        Serial.print(", ");
        Serial.println(envSample.rhPerc);
        formPacketBody();
        updateDisplay();

        // Prepare upstream data transmission at the next possible time.
        LMIC_setTxData2(1, message.uints, strlen(message.envDataStr), 0);
        Serial.println(F("Packet queued"));
    }
    // Next TX is scheduled after TX_COMPLETE event.
}

void scanI2C() {
  byte error, address;
  int nDevices;
  Serial.println(F("Scanning for I2C devices..."));

  nDevices = 0;
  for (address = 0; address <= 127; address++ )
  {
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    //Line below is incorect. Wire lib assumes address is right-shifted
    //use as read and write. This sketch does that and so reports
    // the proper address
    /// ACTUALLY THE SHIFTING IS INCORRECT, AT LEAST FOR Feather M0!!!!
    //Wire.beginTransmission(address>>1);
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0)
    {
      Serial.print(F("I2C device found at address 0x"));
      if (address < 16)
        Serial.print(F("0"));
      Serial.print(address, HEX);
      if (address == 0) {
        Serial.print(F(" (I2C broadcast address some slaves implement and respond to)"));
      }
      Serial.println();

      nDevices++;
    }
    else if (error == 4)
    {
      Serial.print(F("'Other' error at address 0x"));
      if (address < 16)
        Serial.print(F("0"));
      Serial.println(address, HEX);
    }
  }
  if (nDevices == 0)
    Serial.println(F("No I2C devices found\n"));
  else
    Serial.println(F("done\n"));
}

struct EnvSample readEnvData() {
  struct EnvSample sample;
  sample.rhPerc = -999.0;
  sample.tempC = -999.0;

 // read Temp & RH from SHT31
  Serial.println(F("-> readSht31"));
  if (sht.read(false)) {                      // default = true/fast       slow = false
    Serial.println(F("-> read values readSht31"));
    sample.rhPerc = sht.getHumidity();
    sample.tempC = sht.getTemperature();
    Serial.println(F("<- readSht31"));
  } else {
    Serial.print(F("Error in read()\n"));
  }
  return sample;
}


void setup() {

    //Wire.begin([SDA [,SCL]]);
    Wire.begin(4, 15);
    Serial.begin(115200);
    Serial.println(F("Starting"));

    
    sht.begin(SHT31_ADDRESS);

    // Configure OLED by setting the OLED Reset HIGH, LOW, and then back HIGH
    pinMode(OLED_RST, OUTPUT);
    digitalWrite(OLED_RST, HIGH);
    delay(100);
    digitalWrite(OLED_RST, LOW);
    delay(100);
    digitalWrite(OLED_RST, HIGH);

    display.init();
    display.flipScreenVertically();

    display.clear();
    display.setFont(ArialMT_Plain_16);
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.drawString(display.getWidth() / 2, 22, "LoRaWAN");
    //display.setFont(ArialMT_Plain_10);
    display.drawString(display.getWidth() / 2, 42, "T/RH Monitor");
    display.display();
    delay(5000);
  

    scanI2C();

    #ifdef VCC_ENABLE
    // For Pinoccio Scout boards
    pinMode(VCC_ENABLE, OUTPUT);
    digitalWrite(VCC_ENABLE, HIGH);
    delay(1000);
    #endif

    // LMIC init
    os_init();
    // Reset the MAC state. Session and pending data transfers will be discarded.
    LMIC_reset();

    // Start job (sending automatically starts OTAA too)
    do_send(&sendjob);
}

void loop() {
  //unsigned long startMillis;
  //startMillis = millis();
  os_runloop_once();
/*  struct EnvSample envSample = readEnvData();
  Serial.print(envSample.tempC, 2);
  Serial.print(", ");
  Serial.println(envSample.rhPerc);
  startMillis = millis() - startMillis;
  Serial.println(startMillis); */
}
