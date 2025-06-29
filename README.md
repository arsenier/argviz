[![Version](https://img.shields.io/badge/version-v0.0.6-informational)](https://github.com/arsenier/argviz/actions/workflows/auto-semver.yml)

# ArGVIZ

ОСРВ для Arduino с визуализацией графики на терминале.

## Основные функции

- Главный цикл низкоуровневых регуляторов с частотой от 1кГц
- Независимый вывод телеметрии в терминал и на i2c дисплей
- Возможность запуска функций из терминала
- Управление как через терминал, так и через физические кнопки на роботе

## Графический интерфейс

```
┌─────────────────────────┐
│0123456789| 2ms|50ms|.2ms│
│-------------------------│
│LEFT        |RIGHT       │
│enc:    13.4|enc:    10.7│
│w:       0.4|w:       9.2│
│(a) FWD1                 │
│(b) SS90EL               │
│(c) STOP                 │
└─────────────────────────┘
```

Пользователь может задать 10 различных экранов с разной отображаемой информацией.

На одном экране можно отобразить произвольный текст и кнопки управления.

Каждый объект для отображения информации имеет:

- Название
- Отображаемоe значения
- Функцию обработчик нажатия (клик, + и -)

epoch1
===

Имеет только одну колонку объектов.

Курсор имеет координату `y`, показывающую выбранный элемент

Движения вниз-вверх изменяют значение y

y = 0 - никакой объект не выбран

при переходе вправо-влево происходит переключение между экранами

y = 1..6 - выбран объект с номером y

движения вправо-влево вызывают обработчик нажатия для этого объекта


