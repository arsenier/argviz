[![Version](https://img.shields.io/badge/version-v0.1.1-informational)](https://github.com/arsenier/argviz/actions/workflows/auto-semver.yml)

# ArGVIZ - Arduino Guidance VIZual interface

Библиотека для неблокирующего вывода телеметрии в реальном времени с помощью последовательного порта.

## Основные функции

- Возможность вывода и переключения до 10 различных экранов с информацией
- Управление и навигация по клавишам hjkl+space
- Возможность добавления функций обработчиков нажатия кнопок
- Использование не более 10% времени процессора для отображения информации
- Автоматическая подстройка под `void loop()` с любым временем выполнения

## Использование

Для использования библиотеки необходимо использование PlatformIO.

### Установка

Чтобы установить библиотеку в проект добавьте в файл конфигурации проекта `platformio.ini`:

```
lib_deps =
    arsenier/argviz@^x.x.x
```

Актуальную версию библиотеки можно найти в регистре пакетов PlatformIO: https://registry.platformio.org/libraries/arsenier/argviz/installation

### Быстрый старт

Минимальный пример использования библиотеки, использующий все функции:

```c++

#include <Arduino.h>
#include <argviz.h>

uint8_t counter = 0;

SCREEN(screen0,
       {
         ROW("counter [us]: %lu", micros());
         ROW("counter [ms]: %lu", millis());
         CLICK_ROW([](CLICK_STATE state)
                   {
        switch(state)
        {
        case CLICK_LEFT:
            counter--;
            break;
        case CLICK_RIGHT:
            counter++;
            break;
        case CLICK_DOWN:
            counter = 0;
            break;
        default:
            break;
        } }, "click me (h|l|space): %3u", counter);

         ROW("HINT:")
         ROW("control with hjkl+space")
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
```

В этом примере используются все основные функции библиотеки.

После загрузки примера необходимо также добавить некоторые настройки в файл `platformio.ini`:

```
monitor_speed = 115200
monitor_raw = true
```

При запуске монитора последовательного порта будет виден следующий экран:

```
+-------------------------+
|0123456789|23fs| 332| 508|
|-------------------------|
|counter [us]: 151957700  |
|counter [ms]: 151960     |
|click me (h|l|space):   0|
|HINT:                    |
|control with hjkl+space  |
|                         |
+-------------------------+
```

Белым выделением показано положение курсора в экране. Управление курсором происходит с помощью клавиш hjkl как в vim.

```
          ↑
          k         Hint: The `h`{normal} key is at the left and moves left.
     ← h    l →           The `l`{normal} key is at the right and moves right.
         j                The `j`{normal} key looks like a down arrow.
         ↓
```

В argviz есть два примитива из которых формируется экран:

- `ROW` - просто текст
- `CLICK_ROW` - текст с "кнопками"

Обычные строки `ROW` оторбражаются обычным текстом и могут быть выделены, но не кликнуты. В примере это все строки кроме третьей (`click me (h|l|space): 0`).

Третья строка является примером кликабельной строки `CLICK_ROW`. Если она выделена, то при нажатии клавиш управления (влево-вправо-пробел) выполняются действия, прописанные  функцией обработчика нажатия.

В примере это изменение значения счетчика и его сброс.


### Создание простейшего экрана

Для нового экрана используется макрос `SCREEN`. Пример создания простейшего экрана:

```c++
SCREEN(screen0,
       {
        ROW("Hello world!");
       })
```

Для отображения этого экрана в интерфейсе необходимо зарегистрировать его с помощью функции `argviz_registerScreen`:

```c++
void setup()
{
  Serial.begin(115200);

  argviz_init(Serial);
  argviz_registerScreen(0, screen0);
  argviz_start();
}
```

Это добавит экран в список.

### О запуске и работе argviz

Для работы argviz необходимо выполнить следующие действия:

1. Инициализировать последовательный порт (`Serial.begin(115200)`)
2. Инициализировать argviz, указав последовательный порт, который будет использоваться для вывода информации на экране (`argviz_init(Serial)`)
3. Зарегистрировать все необходимые экраны с помощью функции `argviz_registerScreen` (`argviz_registerScreen(0, screen0)`)
4. Запустить argviz (`argviz_start()`)

После этого argviz будет работать параллельно с `void loop()` и выводить информацию на экране.

> [!WARNING]
> программа в `void loop()` не должна блокировать выполнение программы, в противном случае argviz не будет выводить информацию на экране

### Синтаксис строк

В строках можно выводить любую информацию, до тех пор пока она помещается в 25 символов. Для вывода переменных используется форматирование строки с помощью `printf`-подобного формата.

Пример использования синтаксиса строк:

```c++
SCREEN(screen0,
       {
         ROW("counter [us]: %lu", micros());
         ROW("counter [ms]: %lu", millis());
       })
```

Справку по этому синтаксису можно найти в интернете (например https://codelessons.dev/ru/printf-in-c-cplusplus/)

### Кликабельные строки

Кликабельные строки позволяют выделить строку и при нажатии клавиш управления выполнять действия.

Для создания кликабельной строки используется макрос `CLICK_ROW`. Пример создания кликабельной строки:

```c++
SCREEN(screen0,
       {
         CLICK_ROW([](CLICK_STATE state)
                   {
        switch(state)
        {
        case CLICK_LEFT:
            counter--;
            break;
        case CLICK_RIGHT:
            counter++;
            break;
        case CLICK_DOWN:
            counter = 0;
            break;
        default:
            break;
        } }, "click me (h|l|space): %3u", counter);
       })
```

В данном примере функция обработчик задана прямо в теле макроса `CLICK_ROW`. Кроме такого способа можно ее задать как отдельную функцию:

```c++
void clickHandler(CLICK_STATE state)
{
    switch(state)
    {
    case CLICK_LEFT:
        counter--;
        break;
    case CLICK_RIGHT:
        counter++;
        break;
    case CLICK_DOWN:
        counter = 0;
        break;
    default:
        break;
    } }

SCREEN(screen0,
       {
         CLICK_ROW(clickHandler, "click me (h|l|space): %3u", counter);
       })
```

Оба варианта эквивалентны.

## Графический интерфейс

```
+-------------------------+
|0123456789|23fs| 332| 508|
|-------------------------|
|\________/ \__/ \__/ \__/|
| |          |    |     `период вызова void loop() в микросекундах
| |          |     `время выполнения void loop() в микросекундах
| |           `частота обновления интерфейса (кадры в секунду, fps)
|  `список всех экранов; выделением показан текущий экран
|                         |
+-------------------------+
```

