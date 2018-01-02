#ifndef Schlenz_Neopixel_Animations_h
#define Schlenz_Neopixel_Animations_h

#include "Arduino.h"
#include "Adafruit_NeoPixel.h"

class Schlenz_Neopixel_Animations {
  public:
  	int cometSize;
  	int scannerSize;

    Schlenz_Neopixel_Animations(int ledCount, int pin);
    void init();
    void stepCometAnimation(int red, int green, int blue, int white);
    void stepScannerAnimation(int red, int green, int blue, int white);
    void setPixelColor(int i, int r, int g, int b, int w);
    void show();
    void reset();
  private:
    int _pin;
    int _ledCount;
    int _cometFrame;
    int _scannerFrame;
    boolean _scannerForward;
    Adafruit_NeoPixel _strip;

    int adjustColorForDistance(int color, int distance, int featureSize);
    int getPixelFrom(int from, int distance);
};

#endif


