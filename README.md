# AGFX — Advanced Graphics & UI Library

[🇺🇸 English](#en--english) | [🇷🇺 Русский](#ru--русский)

<div align="center">
  <img src="https://img.shields.io/badge/C-00599C?style=for-the-badge&logo=c&logoColor=white" />
  <img src="https://img.shields.io/badge/OSDev-000000?style=for-the-badge&logo=linux&logoColor=white" />
  <img src="https://img.shields.io/badge/Freestanding-Success?style=for-the-badge" />
</div>

---

## EN / English

**AGFX** is a lightweight, completely **freestanding software 2D rendering engine and Immediate Mode GUI (IMGUI) framework** designed for **OSDev**, embedded systems, and bare-metal environments.

It allows you to draw primitives, render TrueType fonts, apply modern visual effects (frosted glass, gradients, soft shadows), and build Windows 10-style user interfaces without relying on a windowing system, GPU APIs, or standard C runtime libraries (`libc`, `libm`, `stdbool.h`).

### Key Features

- **100% Freestanding / No Libc:** Built-in minimal math and string routines. Custom memory allocators (`malloc`/`free`) can be injected via `agfx_set_allocators()`.
- **Pure CPU Software Rasterizer:** Works directly with raw 32-bit ARGB framebuffers (`uint32_t*`).
- **Core 2D Primitives:**
  - Fast pixels, lines (with thickness), quadratic Bezier curves
  - Rectangles (fill, outline, rounded corners)
  - Circles, triangles, convex/concave polygons (fan fill)
  - Blitting with scaling and alpha blending (`agfx_blit`)
  - Strict clipping rectangle support (`clip_x/y/w/h`)
- **Anti-Aliased Mask Engine (`agfx_mask`):**
  - High-quality 8-bit alpha mask generation for complex shapes
  - Boolean mask operations (`SET`, `MAX`, `ADD`, `SUB`)
- **Shaders & Filters (`agfx_filters`):**
  - Smooth linear gradients (standalone or masked onto any shape)
  - Fast box blur
  - Frosted Glass effect (blur + tint) and Acrylic effect
  - Grayscale and color invert
- **Freestanding Typography (`agfx_text`):**
  - Vector TrueType (`.ttf`) font rendering powered by embedded `stb_truetype.h`
  - Generates whole-string alpha masks (supports gradient/acrylic text)
- **Windows 10 Style UI Toolkit (`agfx_ui`):**
  - Immediate Mode GUI (IMGUI) paradigm
  - Theming system: Built-in Dark and Light themes + customizable accent colors
  - Auto-layout engine with `same_line` support
  - Widgets: Buttons, Checkboxes, Radio buttons, Sliders, Progress bars, Textboxes with cursor, Labels, Separators
  - Decoupled `behavior` pattern (`button_behavior`, `slider_behavior`) for infinite visual customization
  - 1-line smooth drop shadows (`agfx_ui_draw_shadow`)

---

### Repository Layout

- **`agfx.h` / `agfx.c`** — Core 2D primitives, clipping, memory hooks, blitting
- **`agfx_mask.h` / `agfx_mask.c`** — 8-bit alpha mask generation & boolean operations
- **`agfx_filters.h` / `agfx_filters.c`** — Image filters, blur, glass, gradients
- **`agfx_text.h` / `agfx_text.c`** — TTF font parsing & string mask generation
- **`agfx_ui.h` / `agfx_ui.c`** — Windows 10 style IMGUI framework & theming
- **`stb_truetype.h`** — Public domain / MIT TrueType rasterizer
- **`test/`** — Linux test harness that generates showcase images (`output_*.bmp`)

---

### Quick Start (Linux / WSL Test)

1. Ensure a TTF font (e.g. `selawk.ttf`) is placed in `test/`.
2. Build and run the test harness:
```bash
cd test
make clean && make run
```

Result images will be generated in `test/`:
- `output_basic.bmp` — Primitives, curves, polygons
- `output_filters.bmp` — Blur, grayscale, invert, glass
- `output_text.bmp` — Solid, gradient, and frosted acrylic typography
- `output_ui.bmp` — Full Windows 10 style compositing demo with overlapping frosted glass dialog

---

### Usage Example (Immediate Mode UI)

```c
#include "agfx.h"
#include "agfx_ui.h"

// 1. Initialize surface and UI context
agfx_surface_t screen;
agfx_init(&screen, framebuffer, width, height, width * 4);
agfx_set_allocators(my_malloc, my_free);

agfx_ui_context_t ui;
agfx_ui_init(&ui, &screen, &my_font);

// 2. Set theme and feed input events from OS
agfx_ui_theme_t theme = agfx_ui_theme_win10_dark();
agfx_ui_set_theme(&ui, &theme);
agfx_ui_set_input(&ui, mouse_x, mouse_y, mouse_down, mouse_clicked);

// 3. Render widgets with automatic layout
agfx_ui_begin(&ui, 20, 20);
agfx_ui_label(&ui, "Settings");
agfx_ui_separator(&ui, 300);

if (agfx_ui_button(&ui, "Apply", 90, 28)) {
    apply_settings();
}
agfx_ui_same_line(&ui);
if (agfx_ui_button(&ui, "Cancel", 90, 28)) {
    close_window();
}

static int dark_mode = 1;
agfx_ui_checkbox(&ui, "Enable Dark Mode", &dark_mode);
```

---

## RU / Русский

**AGFX** — легковесный, полностью **независимый (freestanding) графический 2D-движок и библиотека интерфейса (Immediate Mode GUI)**, разработанный специально для **OSDev**, встраиваемых систем и bare-metal окружений.

Библиотека позволяет рисовать примитивы, рендерить векторные TrueType шрифты, накладывать эффекты матового стекла, градиенты и строить пользовательские интерфейсы в стиле Windows 10 без использования стандартной библиотеки Си (`libc`, `libm`, `stdbool.h`) или графических API операционных систем.

### Основные возможности

- **100% Freestanding (без libc):** Встроенные микро-версии математических и строковых функций. Кастомные аллокаторы памяти ядра или ОС передаются через `agfx_set_allocators()`.
- **Программный CPU-рендеринг:** Прямой вывод в сырой 32-битный ARGB буфер (`uint32_t*`).
- **Базовые 2D-примитивы:**
  - Пиксели, линии с произвольной толщиной, квадратичные кривые Безье
  - Прямоугольники (заливка, контур, скругленные углы)
  - Окружности, треугольники, полигоны
  - Копирование буферов с масштабированием и альфа-смешиванием (`agfx_blit`)
  - Аппаратное/программное отсечение по границам (`agfx_clip_rect`)
- **Генератор сглаженных масок (`agfx_mask`):**
  - Генерация 8-битных альфа-масок для фигур со сглаживанием (Anti-Aliasing)
  - Булевы операции над масками (`SET`, `MAX`, `ADD`, `SUB`)
- **Фильтры и шейдеры (`agfx_filters`):**
  - Линейные градиенты (заливка прямоугольников или любых фигур по маске)
  - Быстрый блочный блюр (Box Blur)
  - Эффект матового стекла (Glass / Acrylic Blur + Tint)
  - Преобразование в оттенки серого (Grayscale) и инверсия цветов
- **Рендеринг шрифтов (`agfx_text`):**
  - Поддержка векторных шрифтов TrueType (`.ttf`) на базе встроенного `stb_truetype.h`
  - Генерация единой альфа-маски для всей строки (позволяет заливать текст градиентом или размывать под ним фон)
- **Фреймворк интерфейса в стиле Windows 10 (`agfx_ui`):**
  - Архитектура Immediate Mode GUI (IMGUI)
  - Система тем: готовые темная и светлая темы Windows 10 + кастомные акцентные цвета
  - Автоматическая компоновка элементов с поддержкой `same_line`
  - Виджеты: Кнопки, Чекбоксы, Радио-кнопки, Слайдеры, Прогресс-бары, Поля ввода текста с курсором, Текст, Разделители
  - Паттерн `behavior` (`button_behavior`, `slider_behavior`) для создания абсолютно любых кастомных кнопок
  - Мягкие тени окон в одну строчку (`agfx_ui_draw_shadow`)

---

### Структура репозитория

- **`agfx.h` / `agfx.c`** — Ядро, базовые примитивы, клиппинг, аллокаторы, блиттинг
- **`agfx_mask.h` / `agfx_mask.c`** — Генерация 8-битных масок и булевы операции
- **`agfx_filters.h` / `agfx_filters.c`** — Фильтры, блюр, матовое стекло, градиенты
- **`agfx_text.h` / `agfx_text.c`** — Парсинг TTF и генерация масок текста
- **`agfx_ui.h` / `agfx_ui.c`** — IMGUI-фреймворк, темы оформления и виджеты
- **`stb_truetype.h`** — Встраиваемый TTF-растеризатор (Public Domain / MIT)
- **`test/`** — Тестовый стенд под Linux с генерацией BMP-изображений

---

### Быстрый запуск тестов (Linux / WSL)

1. Положи любой TTF-шрифт (например `selawk.ttf`) в папку `test/`.
2. Запусти сборку:
```bash
cd test
make clean && make run
```

В папке `test/` появятся 4 файла:
- `output_basic.bmp` — Примитивы, Безье, полигоны
- `output_filters.bmp` — Размытие, оттенки серого, инверсия
- `output_text.bmp` — Сплошной, градиентный и стеклянный текст
- `output_ui.bmp` — Композитинг окон с перекрытием, матовым стеклом и виджетами Windows 10

---

### Лицензия и связанные проекты

- Библиотека распространяется под лицензией **MIT**.
- Модуль растеризации шрифтов использует `stb_truetype.h` (Sean Barrett, Public Domain / MIT).
- Основной проект-потребитель: **[AOS](https://github.com/angeloxhek/AOS)** — кастомная операционная система с нуля.

> *Love and kisses to everyone! I hope you enjoy my projects.* 🤍