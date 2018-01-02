#include "Schlenz_SSD1306_Jenkins.h"
#include "Schlenz_Neopixel_Animations.h"
#include "SoftTimer.h"

#define I2C_ADDRESS 0x3D // I2C Address of SSD1306 OLED Display
#define OLED_RESET_PIN 4 // Pin connected to reset on SSD1306
#define LED_DATA_PIN 6 // Pin to pass data to Neopixels
#define LED_COUNT 60 // Neopixel LED count 
#define COMET_SIZE 10 // Size (number of LEDs) for the comet animation
#define ANIM_DELAY 60 // Delay in millis for LED animations
#define SCANNER_SIZE 21 // Size (number of LEDs) for the scanner animation. NOTE: Should be an odd number
#define STATUS_DIST_DELAY 2000 // Delay between showing the status distribution frames
#define TOGGLE_DISPLAY_MODE_DELAY 2000 // Delay between checks for whether to toggle the display mode
#define TOGGLE_DISPLAY_MODE_DURATION_A 60000000 // Duration of display mode A (overall status animation)
#define TOGGLE_DISPLAY_MODE_DURATION_B 12000000 // Duration of display mode B (status distribution)

// Forward references to functions that will be used for the SoftTimer
void checkForMessage(Task* me);
void animateOverallStatus(Task* me);
void showStatusDistribution(Task* me);
void toggleDisplayMode(Task* me);

Schlenz_Neopixel_Animations _anims(LED_COUNT, LED_DATA_PIN); // Object for handling all Neopixel animations/updates
Schlenz_SSD1306_Jenkins _oled(OLED_RESET_PIN, I2C_ADDRESS);  // Object for handling the parsing of Jenkins status updates from Particle Photon, and displaying on OLED

// Task instances for the SoftTimer
Task checkForMessageTask(0, checkForMessage);
Task animateTask(ANIM_DELAY, animateOverallStatus);
Task showStatusDistributionTask(STATUS_DIST_DELAY, showStatusDistribution);
Task toggleDisplayModeTask(TOGGLE_DISPLAY_MODE_DELAY, toggleDisplayMode);

boolean _allowAnimation = false; // If true, it's safe to step through animations. Otherwise, Serial data is being read which takes priority.
boolean _reset = true; // Whether the Neopixels should be reset before the next animation step
boolean _showingStatusDist = false; // True if we are in display mode B, otherwise display mode A
unsigned long _lastDisplayModeSwitch = 0; // Timestamp for when we last switched the display mode
int _statusDistStep = 0; // Which step (frame) of the status distribution (display mode B) we are on
int _success; // Number of successful Jenkins jobs
int _failure; // Number of failed Jenkins jobs
int _unstable; // Number of unstable Jenkins jobs
int _aborted; // Number of aborted Jenkins jobs
int _notBuilt; // Number of unbuilt Jenkins jobs
int _unknown; // Number of unknown Jenkins jobs

void setup() {         
  // Initialize the Neopixel helper class
  _anims.cometSize = COMET_SIZE;
  _anims.scannerSize = SCANNER_SIZE;
  _anims.init();

  // Initialize the OLED/Jenkins helper class
  _oled.setOnBusyCallback(onBusy);
  _oled.setOnStatusCallback(onStatus);
  _oled.init();

  // Register our SoftTimer tasks
  SoftTimer.add(&checkForMessageTask);
  SoftTimer.add(&toggleDisplayModeTask);
}

// Called by SoftTimer. Time to ask the helper class to check for new serial data.
void checkForMessage(Task* me) {
  _oled.checkForMessage();  
}

// Called by SoftTimer. Execute the next step in the display mode A animation.
void animateOverallStatus(Task* me) {
  if (!_allowAnimation) {
    return;
  }

  if ((_success <= 0) && (_failure <= 0)) {
    // Don't bother doing anything.
    return;
  }

  checkReset();

  if (_failure > 0) {
    _anims.stepScannerAnimation(50, 0, 0, 0);
  } else {
    _anims.stepCometAnimation(0, 50, 0, 0);
  }
}

// Called by SoftTimer. Execute the status distribution frame in display mode B.
void showStatusDistribution(Task *me) {
  if (!_allowAnimation) {
    return;
  }

  float total = (float) (_success + _failure + _unstable + _aborted + _notBuilt + _unknown);

  if (total <= 0) {
    // No jobs, flip back to display mode A.
    toggleDisplayMode(NULL);
    return;
  }

  checkReset();

  int count = 0;
  int r = 0;
  int g = 0;
  int b = 0;
  int w = 0;

  do {
    r = 0;
    g = 0;
    b = 0;
    w = 0;
    
    switch (_statusDistStep) {
      case 0:
        count = _success;
        g = 50;
        break;
      case 1:
        count = _failure;
        r = 50;
        break;
      case 2:
        count = _unstable;
        r = 50;
        g = 50;
        break;
      case 3:
        count = _aborted;
        g = 50;
        b = 50;
        break;
      case 4:
        count = _notBuilt;
        b = 50;
        break;
      case 5:
        count = _unknown;
        w = 50;
        break;
    }
    _statusDistStep++;
  } while ((count == 0) || (_statusDistStep >= 6));

  if (count > 60) {
    count = 60;
  }
  
  _anims.reset();
  for (int i=0; i < count; i++) {
    _anims.setPixelColor(i, r, g, b, w);
  }
  _anims.show();

  if (_statusDistStep >= 6) {
    _statusDistStep = 0;
  }
}

// Called by SoftTimer. Flips between display mode A & B, unless enough time has not passed. 
void toggleDisplayMode(Task *me) {
  unsigned long duration = (micros() - _lastDisplayModeSwitch);
  
  if ((_showingStatusDist && (duration < TOGGLE_DISPLAY_MODE_DURATION_B)) || 
    (!_showingStatusDist && (duration < TOGGLE_DISPLAY_MODE_DURATION_A))) {
    // Not enough time has passed.
    return;
  }
  
  _showingStatusDist = !_showingStatusDist;
  _reset = true;
  _lastDisplayModeSwitch = micros();
  
  if (_showingStatusDist) {
    SoftTimer.add(&showStatusDistributionTask);  
    SoftTimer.remove(&animateTask);
  } else {
    SoftTimer.add(&animateTask);
    SoftTimer.remove(&showStatusDistributionTask);  
  }
}

void checkReset() {
  if (_reset) {
    _reset = false;
    _anims.reset();
  }
}

// Called by Jenkins/OLED helper class. Whether the helper is busy reading serial data.
void onBusy(boolean busy) {
  _allowAnimation = !busy;
}

// Called by Jenkins/OLED helper class. Called when new status data arrives.
void onStatus(int success, int failure, int unstable, int aborted, int notBuilt, int unknown) {
  _success = success;
  _failure = failure;
  _unstable = unstable;
  _aborted = aborted;
  _notBuilt = notBuilt;
  _unknown = unknown;
  
  _showingStatusDist = false;
  _statusDistStep = 0;
  _reset = true;
}

