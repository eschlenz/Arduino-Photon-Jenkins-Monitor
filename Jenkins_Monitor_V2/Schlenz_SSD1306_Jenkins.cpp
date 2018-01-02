#include "Arduino.h"
#include "Adafruit_SSD1306.h"
#include "Schlenz_SSD1306_Jenkins.h"

#define WORD_WAITING          0
#define WORD_INIT             1
#define WORD_TX_RX_CONNECTED  2
#define WORD_NO_COUNT         3
#define WORD_SUCCESS          4
#define WORD_FAILURE          5
#define WORD_UNSTABLE         6
#define WORD_ABORTED          7
#define WORD_NOT_BUILT        8
#define WORD_UNKNOWN          9

const char STRING_0[] PROGMEM  = "Waiting for data...";
const char STRING_1[] PROGMEM  = "INIT"; 
const char STRING_2[] PROGMEM  = "TX/RX connected."; 
const char STRING_3[] PROGMEM  = "000";
const char STRING_4[] PROGMEM  = "SUCCESS  : ";
const char STRING_5[] PROGMEM  = "FAILURE  : ";
const char STRING_6[] PROGMEM  = "UNSTABLE : ";
const char STRING_7[] PROGMEM  = "ABORTED  : ";
const char STRING_8[] PROGMEM  = "NOT BUILT: ";
const char STRING_9[] PROGMEM  = "UNKNOWN  : ";

const char* const STRING_TABLE[] PROGMEM = {STRING_0, STRING_1, STRING_2, STRING_3, STRING_4, STRING_5, STRING_6, STRING_7, STRING_8, STRING_9};

Schlenz_SSD1306_Jenkins::Schlenz_SSD1306_Jenkins(int resetPin, uint8_t i2cAddress) :
  _oled(resetPin) {
  _resetPin = resetPin;
  _i2cAddress = i2cAddress;
}

void Schlenz_SSD1306_Jenkins::setOnBusyCallback(void (*onBusyCallback)(boolean)) {
  _onBusyCallback = onBusyCallback;
}

void Schlenz_SSD1306_Jenkins::setOnStatusCallback(void (*onStatusCallback)(int, int, int, int, int, int)) {
  _onStatusCallback = onStatusCallback;
}

void Schlenz_SSD1306_Jenkins::init() {
  initSerial();
  initOled();
}

void Schlenz_SSD1306_Jenkins::initSerial() {
  Serial1.begin(38400);
}

void Schlenz_SSD1306_Jenkins::initOled() {
  _oled.begin(SSD1306_SWITCHCAPVCC, _i2cAddress); 
  _oled.clearDisplay();
  _oled.setTextSize(1);
  _oled.setTextColor(WHITE, BLACK);
  _oled.setCursor(0,0);
  _oled.display();
 
  readProgmemWord(_progmemBuffer, WORD_WAITING);
  _oled.println(_progmemBuffer); 
  _oled.display();
}

void Schlenz_SSD1306_Jenkins::checkForMessage() {
  while (Serial1.available()) {
    _onBusyCallback(true);

    char received = Serial1.read();

    boolean full = (_serialMsgIndex >= 23);
    boolean newLine = (received == '\n');

    if (_ignoring && !newLine) {
      // Keep reading through the serial data...
      continue;
    } else if (_ignoring && newLine) {
      // Hit a new line, we can stop ignoring data.
      _ignoring = false;
    } else if (full || newLine) {
      if (_serialMsgIndex > 23) {
        _serialMsgIndex = 23;
      }

      _serialMsg[_serialMsgIndex] = '\0';
      processMessage(_serialMsg);
      _serialMsg[0] = '\0';
      _serialMsgIndex = 0;

      if (!newLine) {
        _ignoring = true;
      }
    } else if (_serialMsgIndex < 23) {
      _serialMsg[_serialMsgIndex++] = received;
    }
  }

  _onBusyCallback(false);
}

void Schlenz_SSD1306_Jenkins::processMessage(char *msg) {
  readProgmemWord(_progmemBuffer, WORD_INIT);

  _oled.clearDisplay();
  _oled.setCursor(0,0);
  _oled.display();

  if (strstr(msg, _progmemBuffer) != NULL) {
    // We received an "INIT" from the Particle Photon.
    // Particle device connected to our Arduino.
    readProgmemWord(_progmemBuffer, WORD_TX_RX_CONNECTED);
    _oled.println(_progmemBuffer);
    _oled.display();
    return;
  }

  char status[4];

  int success = printStatus(msg, WORD_SUCCESS, INDEX_SUCCESS);
  int failure = printStatus(msg, WORD_FAILURE, INDEX_FAILURE);
  int unstable = printStatus(msg, WORD_UNSTABLE, INDEX_UNSTABLE);
  int aborted = printStatus(msg, WORD_ABORTED, INDEX_ABORTED);
  int notBuilt = printStatus(msg, WORD_NOT_BUILT, INDEX_NOT_BUILT);
  int unknown = printStatus(msg, WORD_UNKNOWN, INDEX_UNKNOWN);

  _onStatusCallback(success, failure, unstable, aborted, notBuilt, unknown);

  _oled.display();
}

int Schlenz_SSD1306_Jenkins::printStatus(const char *msg, int word, int statusIndex) {
  readProgmemWord(_progmemBuffer, word);
  getStatusCount(_status, msg, statusIndex);
  _oled.print(_progmemBuffer);

  int statusCount = atoi(_status);
  _oled.println(statusCount);

  return statusCount;
}

void Schlenz_SSD1306_Jenkins::getStatusCount(char *status, const char *msg, int index) {
  for (int i=0; i < STATUS_WIDTH; i++) {
    *(status + i) = *(msg + index + i);
  }
  *(status + STATUS_WIDTH) = '\0';
}

void Schlenz_SSD1306_Jenkins::readProgmemWord(char *word, int index) {
  strcpy_P(word, (char*) pgm_read_word(&(STRING_TABLE[index])));
}

