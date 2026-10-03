#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include "epd_driver.h"         // https://github.com/Xinyuan-LilyGO/LilyGo-EPD47

//fonts
#include "Fonts/OpenSans8.h"
#include "Fonts/OpenSansB8.h"
#include "Fonts/OpenSans10.h"
#include "Fonts/OpenSansB10.h"
#include "Fonts/OpenSans12.h"
#include "Fonts/OpenSansB12.h"
#include "Fonts/OpenSans14.h"
#include "Fonts/OpenSansB14.h"
#include "Fonts/OpenSans18.h"
#include "Fonts/OpenSansB18.h"
#include "Fonts/OpenSans24.h"
#include "Fonts/OpenSansB24.h"

#define WHITE         0xFF
#define LIGHT_GREY    0xBB
#define GREY          0x88
#define DARK_GREY     0x44
#define BLACK         0x00

#define DISPLAY_WIDTH 960
#define DISPLAY_HEIGHT 540

class Display {
    public:
    enum alignment {LEFT, RIGHT, CENTER};
    
    static void begin();
    static void clear();
    static void update();

    static void drawArrow(int x, int y, int asize, float aangle, int pwidth, int plength);
    static void drawCircle(int x0, int y0, int r, uint8_t color);
    static void drawFastVLine(int16_t x0, int16_t y0, int length, uint16_t color);
    static void drawFastHLine(int16_t x0, int16_t y0, int length, uint16_t color);
    static void drawGrayScale(uint16_t x, uint16_t y, const uint16_t width, const uint16_t height, const uint8_t *data);
    static void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
    static void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
    static void drawPixel(int x, int y, uint8_t color);
    static void drawSegment(int x, int y, int o1, int o2, int o3, int o4, int o11, int o12, int o13, int o14);
    static void drawString(int32_t x, int32_t y, String text, alignment align);
    static void fillCircle(int x, int y, int r, uint8_t color);
    static void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
    static void fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);
    static void pushPixels(Rect_t rect, uint16_t delay, uint8_t count = 1);
    static void setFont(GFXfont const &font);
    static uint16_t getTextWidth(const String& text);
};

#endif
