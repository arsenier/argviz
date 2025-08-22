#include <Arduino.h>
#include <argviz.h>

/**
Set

```
monitor_raw = true
monitor_speed = 115200
```

in platformio.ini to get proper output
 */

uint8_t counter = 0;

SCREEN(screen0,
       {
           ROW("counter [us]: %lu", micros());
       })

void setup()
{
    Serial.begin(115200);

    argviz_init(Serial);
    argviz_registerScreen(0, screen0);
    argviz_start();
}

void loop()
{
    static uint32_t timer = 0;
    while (micros() - timer < 500)
        ;
    timer = micros();
}
