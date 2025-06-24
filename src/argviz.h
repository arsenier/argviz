#pragma once

#include <Arduino.h>
#include <VT100.h>

#define DISPLAY_WIDTH 25
#define DISPLAY_HEIGHT 8

#define VT100_DISPLAY_WIDTH (DISPLAY_WIDTH + 2)
#define VT100_DISPLAY_HEIGHT (DISPLAY_HEIGHT + 2)

#define SCREENS_AMOUNT 10
#define SCREEN_MAX_HEIGHT (DISPLAY_HEIGHT - 2)

size_t __selectedScreen = 0;
int __selectY = 0;
Stream *__serial_bus;

enum CLICK_STATE
{
    CLICK_NONE,
    CLICK_DOWN,
    CLICK_LEFT,
    CLICK_RIGHT,
};

CLICK_STATE __clickState = CLICK_NONE;

#define ROW(...) \
    { \
        VT100.setCursor(argviz_row + 3, 2); \
        VT100.formatText(VT_RESET); \
        if(__selectY == argviz_row) { \
            VT100.formatText(VT_REVERSE); \
        } \
        char buf[VT100_DISPLAY_WIDTH] = {0}; \
        int len = sprintf(buf, __VA_ARGS__); \
        if(len == -1 || len > DISPLAY_WIDTH) \
        { \
            __serial_bus->print("      sprintf error      "); \
        } \
        else \
        { \
            __serial_bus->print(buf); \
            for(int k = len; k < DISPLAY_WIDTH; k++) \
            { \
                __serial_bus->print(' '); \
            } \
        } \
        argviz_row++; \
    }


#define CLICK_ROW(handlerClick, ...) \
    { \
        VT100.setCursor(argviz_row + 3, 2); \
        VT100.formatText(VT_RESET); \
        VT100.formatText(VT_BRIGHT); \
        if(__selectY == argviz_row) { \
            VT100.formatText(VT_REVERSE); \
            if(__clickState != CLICK_NONE) \
            { \
                handlerClick(__clickState); \
            } \
        } \
        char buf[VT100_DISPLAY_WIDTH] = {0}; \
        int len = sprintf(buf, __VA_ARGS__); \
        if(len == -1 || len > DISPLAY_WIDTH) \
        { \
            __serial_bus->print("      sprintf error      "); \
        } \
        else \
        { \
            __serial_bus->print(buf); \
            for(int k = len; k < DISPLAY_WIDTH; k++) \
            { \
                __serial_bus->print(' '); \
            } \
        } \
        argviz_row++; \
    }

#define SCREEN(name, ...) \
size_t name() \
{ \
    int argviz_row = 1; \
    __VA_ARGS__ \
    int argviz_row_buf = argviz_row; \
    for(; argviz_row <= SCREEN_MAX_HEIGHT;) \
        ROW("                         "); \
    return argviz_row_buf; \
}

size_t __screenBlank()
{
    for(size_t i = 1; i <= 6; i++)
    {
        VT100.setCursor(3 + i, 2);
        __serial_bus->print("                         ");
    }
    return 1;
}

typedef size_t (*screenFunc)();

screenFunc __screens[SCREENS_AMOUNT] = {
    __screenBlank,
    __screenBlank,
    __screenBlank,
    __screenBlank,
    __screenBlank,
    __screenBlank,
    __screenBlank,
    __screenBlank,
    __screenBlank,
    __screenBlank,
};

void argviz_registerScreen(size_t screen_idx, screenFunc func)
{
    __screens[screen_idx] = func;
}

void argviz_unregisterScreen(size_t screen_idx)
{
    __screens[screen_idx] = __screenBlank;
}

void __draw_border()
{
    // Top border
    VT100.setCursor(1, 1);
    __serial_bus->write('+');
    for (int col = 2; col < VT100_DISPLAY_WIDTH; col++) {
        __serial_bus->write('-');
    }
    __serial_bus->write('+');

    // Bottom border
    VT100.setCursor(VT100_DISPLAY_HEIGHT, 1);
    __serial_bus->write('+');
    for (int col = 2; col < VT100_DISPLAY_WIDTH; col++) {
        __serial_bus->write('-');
    }
    __serial_bus->write('+');

    // Side borders
    for (int row = 2; row <= VT100_DISPLAY_HEIGHT - 1; row++) {
        VT100.setCursor(row, 1);
        __serial_bus->write('|');
        VT100.setCursor(row, VT100_DISPLAY_WIDTH);
        __serial_bus->write('|');
    }
}

void __draw_header()
{
    static uint32_t argviz_time0 = micros();

    VT100.setCursor(2, 2);

    for(size_t i = 0; i < SCREENS_AMOUNT; i++)
    {
        if(i == __selectedScreen)
        {
            VT100.formatText(VT_REVERSE);
        }
        else
        {
            VT100.formatText(VT_RESET);
        }
        __serial_bus->print(i);
    }

    VT100.formatText(VT_RESET);
    char buf[20] = {0};
    int32_t dtime = (micros() - argviz_time0) / 1000;
    sprintf(buf, "|%2ldms|  %2d|  %2d", dtime, __selectedScreen, __selectY);
    __serial_bus->print(buf);
    VT100.setCursor(3, 2);
    __serial_bus->print("-------------------------");

    argviz_time0 = micros();
}

void argviz_init(Stream &_serial_bus)
{
    __serial_bus = &_serial_bus;
    VT100.begin(_serial_bus);
    VT100.cursorOff();
}

void argviz_update()
{
    __draw_border();
    __draw_header();

    size_t screenSize = 0;

    switch(__selectedScreen)
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
        screenSize = __screens[__selectedScreen]();
        break;
    default:
        screenSize = __screenBlank();
        break;
    }
    VT100.formatText(VT_RESET);

    __clickState = CLICK_NONE;

    while(__serial_bus->available())
    {
        switch (__serial_bus->read())
        {
        case 'h':
            if(__selectY == 0)
            {
                __selectedScreen = (__selectedScreen + SCREENS_AMOUNT - 1) % SCREENS_AMOUNT;
            }
            else
            {
                __clickState = CLICK_LEFT;
            }
            break;
        case 'l':
            if(__selectY == 0)
            {
                __selectedScreen = (__selectedScreen + 1) % SCREENS_AMOUNT;
            }
            else
            {
                __clickState = CLICK_RIGHT;
            }
            break;
        case 'j':
            __selectY = (__selectY + 1) % screenSize;
            break;
        case 'k':
            __selectY = (__selectY + screenSize - 1) % screenSize;
            break;
        case ' ':
            __clickState = CLICK_DOWN;
            break;
        default:
            break;
        }
    }
}
