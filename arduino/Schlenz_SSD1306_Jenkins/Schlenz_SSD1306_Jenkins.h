#ifndef Schlenz_SSD1306_Jenkins_h
#define Schlenz_SSD1306_Jenkins_h

#include "Arduino.h"
#include "Adafruit_SSD1306.h"

#define OLED_ROW_COUNT 8
#define OLED_HEIGHT 64

#define JENKINS_SUCCESS   0
#define JENKINS_FAILURE   1
#define JENKINS_UNSTABLE  2
#define JENKINS_ABORTED   3
#define JENKINS_NOT_BUILT 4
#define JENKINS_NULL      5

#define INDEX_SUCCESS   0
#define INDEX_FAILURE   4
#define INDEX_UNSTABLE  8
#define INDEX_ABORTED   12
#define INDEX_NOT_BUILT 16
#define INDEX_UNKNOWN   20

#define STATUS_WIDTH 3
#define ROW_MAX_WIDTH 21

class Schlenz_SSD1306_Jenkins {
  public:
    Schlenz_SSD1306_Jenkins(int resetPin, uint8_t i2cAddress);
    void setOnBusyCallback(void (*onBusy)(boolean));
    void setOnStatusCallback(void (*onStatus)(int, int, int, int, int, int));
    void init();
    void checkForMessage();

  private:
    int _resetPin;
    uint8_t _i2cAddress;
    int _serialMsgIndex;
    char _serialMsg[24];
    char _progmemBuffer[22];
    char _status[4];
    boolean _ignoring;
    Adafruit_SSD1306 _oled;    

    void (*_onBusyCallback)(boolean);
    void (*_onStatusCallback)(int, int, int, int, int, int);

    void initSerial();
    void initOled();
    void processMessage(char *msg);
    int printStatus(const char *msg, int word, int statusIndex);
    void getStatusCount(char *status, const char *msg, int index);
    void readProgmemWord(char *word, int index);
};

#endif


