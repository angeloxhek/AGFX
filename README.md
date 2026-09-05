# AGFX — Advanced Graphics Library

[🇺🇸 English](#en--english) | [🇷🇺 Русский](#ru--русский)

<div align="center">
  <img src="https://img.shields.io/badge/C-00599C?style=for-the-badge&logo=c&logoColor=white" />
  <img src="https://img.shields.io/badge/OSDev-000000?style=for-the-badge&logo=linux&logoColor=white" />
  <img src="https://img.shields.io/badge/Software_Rendering-222222?style=for-the-badge" />
</div>

---

## EN / English

**AGFX** is a small, dependency-free **software 2D rendering library** designed for **OSDev** and other low-level projects.

It was built around a simple idea: *draw into a raw pixel buffer without needing a windowing system, GPU APIs, or a standard library.*

### Key Features
- **Pure CPU rendering** (software rasterizer)
- Works with a raw **32-bit framebuffer** (`uint32_t*`)
- **No malloc required** (you provide buffers)
- **Clipping rectangle** support (`clip_x/clip_y/clip_w/clip_h`)
- **Alpha blending** (ARGB style, per-pixel alpha)
- Common primitives & shapes:
  - pixel / line (with thickness)
  - Bezier curve (quadratic)
  - rectangles (fill / outline / rounded)
  - circles (fill / outline)
  - triangles (fill / outline)
  - polygons (outline + fan-based fill)
- Optional “extras” module with **gradients** and non-core shapes

### Repository Layout
- **`agfx.h` / `agfx.c`** — core primitives (solid fills, blending, clip)
- **`agfx_ext.h` / `agfx_ext.c`** — extended stuff (gradients, fancy shapes, etc.)
- **`test/`** — linux/WSL test harness that renders to `output_*.bmp`

### Quick Start (Linux / WSL)
```bash
cd test
make clean && make run
```

The result image will be written into `test/` (e.g. `output_showcase.bmp`).

### Usage Example
```c
#include "agfx.h"

agfx_surface_t s;
agfx_init(&s, framebuffer, width, height, width * 4);
agfx_set_clip(&s, 0, 0, width, height);

agfx_fill_rect(&s, 0, 0, width, height, 0xFF112233);
agfx_draw_line(&s, 10, 10, 300, 200, 3, 0xFFFFAA00);
agfx_fill_circle(&s, 200, 150, 40, 0x8800FF00);
```

### Notes / Assumptions
- Pixel format is expected to be **32-bit** (`uint32_t` per pixel).  
- `pitch` is **bytes per row** (stride).  
- Blending uses integer math (no floating point required).

### Why?
AGFX is primarily used as a graphics layer for my OSDev projects (especially **AOS**), but it’s kept generic so you can embed it anywhere you can access a framebuffer.

### Related Projects
- **[AOS](https://github.com/angeloxhek/AOS)** — my custom OS from scratch

> *Love and kisses to everyone! I hope you enjoy my projects.* 🤍

---

## RU / Русский

**AGFX** — маленькая, независимая от окружения **2D библиотека программного рендеринга**, сделанная с прицелом на **OSDev** и низкоуровневые проекты.

Идея простая: *рисовать напрямую в буфер пикселей, без окон, без GPU API, без тяжёлых зависимостей.*

### Возможности
- **CPU-only** рендеринг (software rasterizer)
- Рисование в сырой **32-битный framebuffer** (`uint32_t*`)
- **Без malloc** (буферы задаёшь сам)
- Поддержка **области отсечения (clip rect)**
- **Альфа-смешивание** (ARGB, прозрачность на пиксель)
- Примитивы и фигуры:
  - пиксель / линия (с толщиной)
  - кривая Безье (квадратичная)
  - прямоугольники (заливка / контур / скруглённые)
  - окружности (заливка / контур)
  - треугольники (заливка / контур)
  - многоугольники (контур + заливка “веером”)
- Отдельный модуль расширений: **градиенты** и нестандартные штуки

### Структура репозитория
- **`agfx.h` / `agfx.c`** — ядро (solid-рендер, blending, clip)
- **`agfx_ext.h` / `agfx_ext.c`** — расширения (градиенты, “лишние” фигуры и т.п.)
- **`test/`** — тестовый стенд под Linux/WSL, вывод в `output_*.bmp`

### Быстрый запуск (Linux / WSL)
```bash
cd test
make clean && make run
```

Результат будет сохранён в `test/` (например `output_showcase.bmp`).

### Пример использования
```c
#include "agfx.h"

agfx_surface_t s;
agfx_init(&s, framebuffer, width, height, width * 4);
agfx_set_clip(&s, 0, 0, width, height);

agfx_fill_rect(&s, 0, 0, width, height, 0xFF112233);
agfx_draw_line(&s, 10, 10, 300, 200, 3, 0xFFFFAA00);
agfx_fill_circle(&s, 200, 150, 40, 0x8800FF00);
```

### Примечания
- Ожидается **32-bit** формат пикселя (`uint32_t` на пиксель)
- `pitch` — это **байт на строку** (stride)
- Вся математика — **целочисленная**, без `float`

### Зачем?
AGFX в первую очередь пишется как графический слой для моих OSDev проектов (особенно **AOS**), но остаётся максимально универсальной библиотекой для любых буферов пикселей.

### Связанные проекты
- **[AOS](https://github.com/angeloxhek/AOS)** — моя ОС с нуля

> *Всех люблю, всех целую! Надеюсь, вам понравятся мои проекты.* 🤍