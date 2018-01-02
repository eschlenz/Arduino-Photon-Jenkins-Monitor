#include "Schlenz_SSD1306_Jenkins.h"
#include "Schlenz_Neopixel_Animations.h"
#include "SoftTimer.h"

#define I2C_ADDRESS 0x3D
#define OLED_RESET_PIN 4
#define LED_DATA_PIN 6
#define LED_COUNT 60
#define COMET_SIZE 10
#define ANIM_DELAY 60
#define SCANNER_SIZE 21 // Should be an odd number
#define STATUS_DIST_DELAY 2000
#define TOGGLE_DISPLAY_MODE_DELAY 2000
#define TOGGLE_DISPLAY_MODE_DURATION_A 60000000
#define TOGGLE_DISPLAY_MODE_DURATION_B 12000000

void checkForMessage(Task* me);
void animateOverallStatus(Task* me);
void showStatusDistribution(Task* me);
void toggleDisplayMode(Task* me);

Schlenz_Neopixel_Animations _anims(LED_COUNT, LED_DATA_PIN);
Schlenz_SSD1306_Jenkins _oled(OLED_RESET_PIN, I2C_ADDRESS);

Task checkForMessageTask(0, checkForMessage);
Task animateTask(ANIM_DELAY, animateOverallStatus);
Task showStatusDistributionTask(STATUS_DIST_DELAY, showStatusDistribution);
Task toggleDisplayModeTask(TOGGLE_DISPLAY_MODE_DELAY, toggleDisplayMode);

boolean _allowAnimation = false;
boolean _reset = true;
boolean _showingStatusDist = false;
unsigned long _lastDisplayModeSwitch = 0;
int _statusDistStep = 0;
int _success;
int _failure;
int _unstable;
int _aborted;
int _notBuilt;
int _unknown;

void setup() {         
  _anims.cometSize = COMET_SIZE;
  _anims.scannerSize = SCANNER_SIZE;
  _anims.init();

  _oled.setOnBusyCallback(onBusy);
  _oled.setOnStatusCallback(onStatus);
  _oled.init();

  SoftTimer.add(&checkForMessageTask);
  SoftTimer.add(&toggleDisplayModeTask);
}

void checkForMessage(Task* me) {
  _oled.checkForMessage();  
}

void animateOverallStatus(Task* me) {
  if (!_allowAnimation) {
    return;
  }

  if ((_success <= 0) && (_failure <= 0)) {
    return;
  }

  checkReset();

  if (_failure > 0) {
    _anims.stepScannerAnimation(50, 0, 0, 0);
  } else {
    _anims.stepCometAnimation(0, 50, 0, 0);
  }
}

void showStatusDistribution(Task *me) {
  if (!_allowAnimation) {
    return;
  }

  float total = (float) (_success + _failure + _unstable + _aborted + _notBuilt + _unknown);

  if (total <= 0) {
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

void toggleDisplayMode(Task *me) {
  unsigned long duration = (micros() - _lastDisplayModeSwitch);
  
  if ((_showingStatusDist && (duration < TOGGLE_DISPLAY_MODE_DURATION_B)) || 
    (!_showingStatusDist && (duration < TOGGLE_DISPLAY_MODE_DURATION_A))) {
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

void onBusy(boolean busy) {
  _allowAnimation = !busy;
}

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

