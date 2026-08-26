#include "Display.h"

int currentScreen = 0, currentSelection = 0, 
scanZCCount = 0, scanRCCount = 0, vref = 1100;
GFXfont  currentFont;
uint8_t *framebuffer;
uint8_t *framebufferflip;
uint16_t refreshCount = 0;

// Display commands
void Display::begin() {
    epd_init();
    framebuffer = (uint8_t *)ps_calloc(sizeof(uint8_t), EPD_WIDTH * EPD_HEIGHT / 2);
    framebufferflip = (uint8_t *)ps_calloc(sizeof(uint8_t), EPD_WIDTH * EPD_HEIGHT / 2);
    memset(framebuffer, 0xFF, EPD_WIDTH * EPD_HEIGHT / 2);
    memset(framebufferflip, 0xFF, EPD_WIDTH * EPD_HEIGHT / 2);
    refreshCount = 0;
}
void Display::update() {
    refreshCount++;
    // Rotate the framebuffer 180 degrees for screen rotation
    for (int y = 0; y < EPD_HEIGHT; y++) {
        for (int x = 0; x < EPD_WIDTH / 2; x++) {
            int src_idx = y * (EPD_WIDTH / 2) + x;
            int dst_idx = (EPD_HEIGHT - 1 - y) * (EPD_WIDTH / 2) + x;
            framebufferflip[dst_idx] = framebuffer[src_idx];
        }
    }
    // Swap buffers
    epd_poweron();
    epd_draw_grayscale_image(epd_full_screen(), framebufferflip); // Update the screen
    epd_poweroff();
}
void Display::clear() {
    epd_poweron();
    epd_clear();
    memset(framebuffer, WHITE, EPD_WIDTH * EPD_HEIGHT / 2);
    memset(framebufferflip, WHITE, EPD_WIDTH * EPD_HEIGHT / 2);
    refreshCount = 0;
    epd_poweroff();
}

// Base Draw Functions
void Display::drawArrow(int x, int y, int asize, float aangle, int pwidth, int plength) {
    // x,y is the centre poistion of the arrow and asize is the radius out from the x,y position
    // aangle is angle to draw the pointer at e.g. at 45° for NW
    // pwidth is the pointer width in pixels
    // plength is the pointer length in pixels
    float dx = (asize - 10) * cos((aangle - 90) * PI / 180) + x; // calculate X position
    float dy = (asize - 10) * sin((aangle - 90) * PI / 180) + y; // calculate Y position
    float x1 = 0;         float y1 = plength;
    float x2 = pwidth / 2;  float y2 = pwidth / 2;
    float x3 = -pwidth / 2; float y3 = pwidth / 2;
    float angle = aangle * PI / 180 - 135;
    float xx1 = x1 * cos(angle) - y1 * sin(angle) + dx;
    float yy1 = y1 * cos(angle) + x1 * sin(angle) + dy;
    float xx2 = x2 * cos(angle) - y2 * sin(angle) + dx;
    float yy2 = y2 * cos(angle) + x2 * sin(angle) + dy;
    float xx3 = x3 * cos(angle) - y3 * sin(angle) + dx;
    float yy3 = y3 * cos(angle) + x3 * sin(angle) + dy;
    fillTriangle(xx1, yy1, xx3, yy3, xx2, yy2, BLACK);
}
void Display::drawCircle(int x0, int y0, int r, uint8_t color) {
    x0 = DISPLAY_WIDTH - x0;
    epd_draw_circle(x0, y0, r, color, framebuffer);
}
void Display::drawFastHLine(int16_t x0, int16_t y0, int length, uint16_t color) {
    x0 = DISPLAY_WIDTH - x0 - length;
    epd_draw_hline(x0, y0, length, color, framebuffer);
}
void Display::drawFastVLine(int16_t x0, int16_t y0, int length, uint16_t color) {
    x0 = DISPLAY_WIDTH - x0;
    epd_draw_vline(x0, y0, length, color, framebuffer);
}
void Display::drawGrayScale(uint16_t x, uint16_t y, const uint16_t width, const uint16_t height, const uint8_t *data) {
    // Draw a grayscale image at position (x, y) with specified width and height
    epd_poweron();
    Rect_t rect = {.x = x, .y = y, .width = width, .height = height};
    epd_draw_grayscale_image(rect, (uint8_t *) data);
    epd_poweroff();
}
void Display::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {
    x0 = DISPLAY_WIDTH - x0;
    x1 = DISPLAY_WIDTH - x1;
    epd_write_line(x0, y0, x1, y1, color, framebuffer);
}
void Display::drawPixel(int x, int y, uint8_t color) {
    x = DISPLAY_WIDTH - x;
    epd_draw_pixel(x, y, color, framebuffer);
}
void Display::drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    x = DISPLAY_WIDTH - x;
    epd_draw_rect(x, y, w, h, color, framebuffer);
}
void Display::drawSegment(int x, int y, int o1, int o2, int o3, int o4, int o11, int o12, int o13, int o14) {
    x = DISPLAY_WIDTH - x;
    drawLine(x + o1,  y + o2,  x + o3,  y + o4,  BLACK);
    drawLine(x + o11, y + o12, x + o13, y + o14, BLACK);
}
void Display::drawString(int32_t x, int32_t y, String text, alignment align) {
    char * data  = const_cast<char*>(text.c_str());
    x = DISPLAY_WIDTH - x;
    int32_t x1, y1, w, h, xx = x, yy = y;
    // Reverse the string
    int len = text.length();
    String reversedText = "";
    for (int i = len - 1; i >= 0; i--) {
        reversedText += text[i];
    }
    data = const_cast<char*>(reversedText.c_str());
    get_text_bounds(&currentFont, data, &xx, &yy, &x1, &y1, &w, &h, NULL);

    if (align == LEFT)  x -= w;
    if (align == CENTER) x -= w / 2;

    int32_t cursor_y = y;
    write_string(&currentFont, data, &x, &cursor_y, framebuffer);
}
void Display::fillCircle(int x, int y, int r, uint8_t color) {
    x = DISPLAY_WIDTH - x;
    epd_fill_circle(x, y, r, color, framebuffer);
}
void Display::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    x = DISPLAY_WIDTH - x - w;
    epd_fill_rect(x, y, w, h, color, framebuffer);
}
void Display::fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color) {
    x0 = DISPLAY_WIDTH - x0;
    x1 = DISPLAY_WIDTH - x1;
    x2 = DISPLAY_WIDTH - x2;
    epd_fill_triangle(x0, y0, x1, y1, x2, y2, color, framebuffer);
}
void Display::pushPixels(Rect_t rect, uint16_t delay, uint8_t count) {
    refreshCount++;
    if (refreshCount >= 255) {
        // Force a full refresh every 10 updates to prevent ghosting
        epd_poweron();
        epd_clear();
        update();
        epd_poweroff();
        refreshCount = 0;
    }
    else {
        rect.x = DISPLAY_WIDTH - rect.x - rect.width;
        rect.y = DISPLAY_HEIGHT - rect.y - rect.height;
        epd_poweron();
        epd_push_pixels(rect, delay, count);
        epd_poweroff();
    }
}
void Display::setFont(GFXfont const &font) {
    currentFont = font;
}
uint16_t Display::getTextWidth(const String& text) {
    int32_t x1, y1, w, h, x = 0, y = 0;
    char* data = const_cast<char*>(text.c_str());
    get_text_bounds(&currentFont, data, &x, &y, &x1, &y1, &w, &h, NULL);
    if (w < 0) {
        return 0;
    }
    return static_cast<uint16_t>(w);
}