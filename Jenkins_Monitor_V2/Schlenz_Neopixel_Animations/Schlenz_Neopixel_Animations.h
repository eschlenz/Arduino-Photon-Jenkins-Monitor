#ifndef Schlenz_Neopixel_Animations_h
#define Schlenz_Neopixel_Animations_h

#include "Arduino.h"
#include "Adafruit_NeoPixel.h"

class Schlenz_Neopixel_Animations {
  public:
  	int cometSize; // Desired size of the comet tail animation (# of LEDs)
  	int scannerSize; // Desired size of the scanner animation (# of LEDs). NOTE: Should be an odd number

    Schlenz_Neopixel_Animations(int ledCount, int pin);
    void init(); // Initialize the Neopixels
    void stepCometAnimation(int red, int green, int blue, int white); // Execute the next step in the comet animation
    void stepScannerAnimation(int red, int green, int blue, int white); // Execute the next step in the scanner animation
    void setPixelColor(int i, int r, int g, int b, int w); // Set a specific pixel's color value
    void show(); // Flush changes to the Neopixels
    void reset(); // Whipe out all Neopixels so they show nothing
  private:
    int _pin; // The pin for passing data to the Neopixels
    int _ledCount; // The number of Neopixels chained together
    int _cometFrame; // What frame (step) we are on in the comet animation
    int _scannerFrame; // What frame (step) we are on in the scanner animation
    boolean _scannerForward; // True of the scanner animation is growing, otherwise it's shrinking
    Adafruit_NeoPixel _strip; // Reference to the NeoPixel object

    int adjustColorForDistance(int color, int distance, int featureSize); // Set's a color to a value inversely proportional to the distance
    int getPixelFrom(int from, int distance); // Returns the index of a pixel relative to the one specified
};

#endif



