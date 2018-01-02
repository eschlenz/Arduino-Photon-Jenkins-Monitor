#include "Arduino.h"
#include "Adafruit_NeoPixel.h"
#include "Schlenz_Neopixel_Animations.h"

Schlenz_Neopixel_Animations::Schlenz_Neopixel_Animations(int ledCount, int pin) {
  _ledCount = ledCount;
  _pin = pin;
  _scannerForward = true;
}

void Schlenz_Neopixel_Animations::init() {
  _strip = Adafruit_NeoPixel(_ledCount, _pin, NEO_GRBW + NEO_KHZ800);  
  _strip.begin();
  _strip.show(); // Initialize all pixels to 'off'
}

void Schlenz_Neopixel_Animations::stepCometAnimation(int red, int green, int blue, int white) {
  if ((_cometFrame < 1) || (_cometFrame >= _ledCount)) {
    _cometFrame = 1;
  } else {
    _cometFrame++;
  }

  for (int i=0; i < _ledCount; i++) {
    int distance;
    
    if (i <= _cometFrame) {
      distance = (_cometFrame - i);
    } else {
      distance = (_ledCount + (_cometFrame - i));
    }
    
    int adjRed = adjustColorForDistance(red, distance, cometSize);
    int adjGreen = adjustColorForDistance(green, distance, cometSize);
    int adjBlue = adjustColorForDistance(blue, distance, cometSize);
    int adjWhite = adjustColorForDistance(white, distance, cometSize);
    
    _strip.setPixelColor(i, adjRed, adjGreen, adjBlue, adjWhite);      
  }
  
  _strip.show();
}

void Schlenz_Neopixel_Animations::stepScannerAnimation(int red, int green, int blue, int white) {
  int halfScannerSize = (scannerSize / 2) + 1;
  int halfPoint = (_ledCount / 2);

  if (_scannerForward) {
    int neighborCount = _scannerFrame;
    
    _strip.setPixelColor(0, red, green, blue, white);
    _strip.setPixelColor(halfPoint, red, green, blue, white);
    
    for (int i=0; i < neighborCount; i++) {
      int toLeft = getPixelFrom(0, (i + 1));
      int toRight = getPixelFrom(0, ((i + 1) * -1));
      
      _strip.setPixelColor(toLeft, red, green, blue, white);
      _strip.setPixelColor(toRight, red, green, blue, white);
      
      toLeft = getPixelFrom(halfPoint, (i + 1));
      toRight = getPixelFrom(halfPoint, ((i + 1) * -1));
      
      _strip.setPixelColor(toLeft, red, green, blue, white);
      _strip.setPixelColor(toRight, red, green, blue, white);
    }
    
    _strip.show();
    _scannerFrame++;
  } else {
    int neighborCount = _scannerFrame;
    
    _strip.setPixelColor(0, red, green, blue, white);
    _strip.setPixelColor(halfPoint, red, green, blue, white);
    
    for (int i=halfScannerSize; i >= _scannerFrame; i--) {
      int toLeft = getPixelFrom(0, (i + 1));
      int toRight = getPixelFrom(0, ((i + 1) * -1));
      
      _strip.setPixelColor(toLeft, 0, 0, 0, 0);
      _strip.setPixelColor(toRight, 0, 0, 0, 0);
      
      toLeft = getPixelFrom(halfPoint, (i + 1));
      toRight = getPixelFrom(halfPoint, ((i + 1) * -1));
      
      _strip.setPixelColor(toLeft, 0, 0, 0, 0);
      _strip.setPixelColor(toRight, 0, 0, 0, 0);
    }
    
    _strip.show();
    _scannerFrame--;
  }

  if (_scannerFrame < 0) {
    _scannerFrame = 0;
    _scannerForward = true;
  } else if (_scannerFrame >= halfScannerSize) {
    _scannerFrame = halfScannerSize;
    _scannerForward = false;
  }
}

void Schlenz_Neopixel_Animations::setPixelColor(int i, int r, int g, int b, int w) {
  _strip.setPixelColor(i, r, g, b, w);
}

void Schlenz_Neopixel_Animations::show() {
  _strip.show();
}

void Schlenz_Neopixel_Animations::reset() {
  for (int i=0; i < _ledCount; i++) {
    _strip.setPixelColor(i, 0, 0, 0, 0);
    _strip.show();
  }
}

int Schlenz_Neopixel_Animations::adjustColorForDistance(int color, int distance, int featureSize) {
  distance = abs(distance);
  
  float perc;
        
  if (distance <= featureSize) {
    perc = ((float) .5 * ((float) featureSize - (float) distance) / (float) featureSize);
  } else {
    perc = 0;
  }

  
  return (int) (perc * (float) color);
}

int Schlenz_Neopixel_Animations::getPixelFrom(int from, int distance) {
  int pixel = (from + distance);
  
  if ((pixel < 0) || (pixel > (_ledCount - 1))) {
    pixel = ((pixel + _ledCount) % _ledCount);
  }
  
  return pixel;
}
