#pragma once

#include <Arduino.h>
#include <VT100.h>

#define DISPLAY_WIDTH 25
#define DISPLAY_HEIGHT 8

#define VT100_DISPLAY_WIDTH (DISPLAY_WIDTH + 2)
#define VT100_DISPLAY_HEIGHT (DISPLAY_HEIGHT + 2)

#define SCREENS_AMOUNT 10
#define SCREEN_MAX_HEIGHT (DISPLAY_HEIGHT - 2)

size_t selectedScreen = 0;
int selectY = 0;

enum CLICK_STATE
{
    CLICK_NONE,
    CLICK_DOWN,
    CLICK_LEFT,
    CLICK_RIGHT,
};

CLICK_STATE clickState = CLICK_NONE;

#define ROW(...) \
    { \
        VT100.setCursor(argviz_row + 3, 2); \
        VT100.formatText(VT_RESET); \
        if(selectY == argviz_row) { \
            VT100.formatText(VT_REVERSE); \
        } \
        char buf[VT100_DISPLAY_WIDTH] = {0}; \
        int len = sprintf(buf, __VA_ARGS__); \
        if(len == -1 || len > DISPLAY_WIDTH) \
        { \
            Serial.print("      sprintf error      "); \
        } \
        else \
        { \
            Serial.print(buf); \
            for(int k = len; k < DISPLAY_WIDTH; k++) \
            { \
                Serial.print(' '); \
            } \
        } \
        argviz_row++; \
    }


#define CLICK_ROW(handlerClick, ...) \
    { \
        VT100.setCursor(argviz_row + 3, 2); \
        VT100.formatText(VT_RESET); \
        VT100.formatText(VT_BRIGHT); \
        if(selectY == argviz_row) { \
            VT100.formatText(VT_REVERSE); \
            if(clickState != CLICK_NONE) \
            { \
                handlerClick(clickState); \
            } \
        } \
        char buf[VT100_DISPLAY_WIDTH] = {0}; \
        int len = sprintf(buf, __VA_ARGS__); \
        if(len == -1 || len > DISPLAY_WIDTH) \
        { \
            Serial.print("      sprintf error      "); \
        } \
        else \
        { \
            Serial.print(buf); \
            for(int k = len; k < DISPLAY_WIDTH; k++) \
            { \
                Serial.print(' '); \
            } \
        } \
        argviz_row++; \
    }

#define SCREEN(idx, ...) \
size_t screen##idx() \
{ \
    size_t argviz_row = 1; \
    __VA_ARGS__ \
    size_t argviz_row_buf = argviz_row; \
    for(; argviz_row <= SCREEN_MAX_HEIGHT;) \
        ROW("                         "); \
    return argviz_row_buf; \
}

size_t screenBlank()
{
    for(size_t i = 1; i <= 6; i++)
    {
        VT100.setCursor(3 + i, 2);
        Serial.print("                         ");
    }
    return 1;
}

typedef size_t (*screenFunc)();

screenFunc screens[SCREENS_AMOUNT] = {
    screenBlank,
    screenBlank,
    screenBlank,
    screenBlank,
    screenBlank,
    screenBlank,
    screenBlank,
    screenBlank,
    screenBlank,
    screenBlank,
};

void registerScreen(size_t screen_idx, screenFunc func)
{
    screens[screen_idx] = func;
}

void agz_draw_border()
{
    // Top border
    VT100.setCursor(1, 1);
    Serial.write('+');
    for (int col = 2; col < VT100_DISPLAY_WIDTH; col++) {
        Serial.write('-');
    }
    Serial.write('+');

    // Bottom border
    VT100.setCursor(VT100_DISPLAY_HEIGHT, 1);
    Serial.write('+');
    for (int col = 2; col < VT100_DISPLAY_WIDTH; col++) {
        Serial.write('-');
    }
    Serial.write('+');

    // Side borders
    for (int row = 2; row <= VT100_DISPLAY_HEIGHT - 1; row++) {
        VT100.setCursor(row, 1);
        Serial.write('|');
        VT100.setCursor(row, VT100_DISPLAY_WIDTH);
        Serial.write('|');
    }
}

void agz_draw_header()
{
    static uint32_t argviz_time0 = micros();

    VT100.setCursor(2, 2);

    for(size_t i = 0; i < SCREENS_AMOUNT; i++)
    {
        if(i == selectedScreen)
        {
            VT100.formatText(VT_REVERSE);
        }
        else
        {
            VT100.formatText(VT_RESET);
        }
        Serial.print(i);
    }

    VT100.formatText(VT_RESET);
    char buf[20] = {0};
    int32_t dtime = (micros() - argviz_time0) / 1000;
    sprintf(buf, "|%2ldms|  %2d|  %2d", dtime, selectedScreen, selectY);
    Serial.print(buf);
    VT100.setCursor(3, 2);
    Serial.print("-------------------------");

    argviz_time0 = micros();
}

void agz_init()
{
    Serial.begin(1000000);
    VT100.begin(Serial);
    VT100.cursorOff();
}

void agz_update()
{
    agz_draw_border();
    agz_draw_header();

    size_t screenSize = 0;

    switch(selectedScreen)
    {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
        screenSize = screens[selectedScreen]();
        break;
    default:
        screenSize = screenBlank();
        break;
    }
    VT100.formatText(VT_RESET);

    clickState = CLICK_NONE;

    while(Serial.available())
    {
        switch (Serial.read())
        {
        case 'h':
            if(selectY == 0)
            {
                selectedScreen = (selectedScreen + SCREENS_AMOUNT - 1) % SCREENS_AMOUNT;
            }
            else
            {
                clickState = CLICK_LEFT;
            }
            break;
        case 'l':
            if(selectY == 0)
            {
                selectedScreen = (selectedScreen + 1) % SCREENS_AMOUNT;
            }
            else
            {
                clickState = CLICK_RIGHT;
            }
            break;
        case 'j':
            selectY = (selectY + 1) % screenSize;
            break;
        case 'k':
            selectY = (selectY + screenSize - 1) % screenSize;
            break;
        case ' ':
            clickState = CLICK_DOWN;
            break;
        default:
            break;
        }
    }
}
