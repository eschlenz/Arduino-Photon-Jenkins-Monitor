#ifndef Schlenz_SSD1306_Jenkins_h
#define Schlenz_SSD1306_Jenkins_h

#include "Arduino.h"
#include "Adafruit_SSD1306.h"

#define INDEX_SUCCESS   0   // Index of success count in formatted Jenkins status payload
#define INDEX_FAILURE   4   // Index of failure count in formatted Jenkins status payload
#define INDEX_UNSTABLE  8   // Index of unstable count in formatted Jenkins status payload
#define INDEX_ABORTED   12  // Index of aborted count in formatted Jenkins status payload
#define INDEX_NOT_BUILT 16  // Index of unbuilt count in formatted Jenkins status payload
#define INDEX_UNKNOWN   20  // Index of unknown count in formatted Jenkins status payload

#define STATUS_WIDTH 3      // Number of digits representing a status count in Jenkins status payload
#define ROW_MAX_WIDTH 21    // Max number of characters per line we can display on the SSD1306

class Schlenz_SSD1306_Jenkins {
  public:
    Schlenz_SSD1306_Jenkins(int resetPin, uint8_t i2cAddress);
    void setOnBusyCallback(void (*onBusy)(boolean)); // Specify a callback function to be notified in "busy" state changes
    void setOnStatusCallback(void (*onStatus)(int, int, int, int, int, int)); // Specify a callback function to be notified of the Jenkins status counts
    void init(); // Initialize this class
    void checkForMessage(); // Check for new serial data

  private:
    int _resetPin; // The pin connected to reset on the SSD1306
    uint8_t _i2cAddress; // The I2C address of the SSD1306
    int _serialMsgIndex; // A cursor to a character index within a Jenkins status payload
    char _serialMsg[24]; // Buffer to hold Jenkins status payload
    char _progmemBuffer[22]; // Buffer to hold strings coming from PROGMEM
    char _status[4]; // Buffer to hold a specific status count
    boolean _ignoring; // Whether characters are being ignored until a new line is reached
    Adafruit_SSD1306 _oled; // A reference to the OLED object

    void (*_onBusyCallback)(boolean); // Reference to an onBusy callback function
    void (*_onStatusCallback)(int, int, int, int, int, int); // Reference to an onStatus callback function

    void initSerial(); // Init the serial connection
    void initOled(); // Init the SSD1306
    void processMessage(char *msg); // Parse the last Jenkins status payload received
    int printStatus(const char *msg, int word, int statusIndex); // Show a status count on the SSD1306
    void getStatusCount(char *status, const char *msg, int index); // Parse a specific status count into a buffer
    void readProgmemWord(char *word, int index); // Pull a string out of PROGMEM and into a buffer
};

#endif



