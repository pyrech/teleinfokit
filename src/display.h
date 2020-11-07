#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include <ESP8266WiFi.h> //https://github.com/esp8266/Arduino
// For a connection via I2C using the Arduino Wire include:
#include <Wire.h>        // Only needed for Arduino 1.6.5 and earlier
#include "SSD1306Wire.h" // legacy: #include "SSD1306.h"
#include "data.h"


#define HEIGHT  32
#define WIDTH   128

class Display
{
public:
    Display();

    void init(Data * d);
    void loop(void);
    void log(String text, int16_t displayTime = 500);
    void logPercent(String text, int percentage);
    void drawGraph();
    void displayData1(long papp, long iinst);
    void displayData2(long hp, long hc);
    void displayData3(char* adc0, long isousc, char* ptec);
    void displayNetwork();
    void displayReset();
    void displayOff();

private:
    Data *data;
};

#endif /* DISPLAY_H */