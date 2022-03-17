# Simple 2d game engine

Cubebox features simple toy educational engine.

# Features
- 2d image batch drawing
- 2d sprite animation
- truetype font caching
- socket support
- tilemap supports

# Third party libraries

- linmath.h - https://github.com/datenwolf/linmath.h
- glad.c, glad.h - https://github.com/Dav1dde/glad
- stb_image.h, stb_rect_pack.h, stb_truetype.h, - stretchy_buffer.h - https://github.com/nothings/stb
- tinyctrhread.c, tinycthread.h - https://github.com/tinycthread/tinycthread

# Ports

- cubebox folder contains full engine divided by files ported to Linux.

These ports are not complete replacement. Only examples.

- cubebox_windows.h - single header window initializing library for Windows, features OpenGL 3.3 with GLAD
- cubebox_android.h - simple android engine with advanced fopen for support android assets and data files

# TODO

Maybe make single header cubebox.h with OpenGL initializing and basic utils supported by Windows/Linux/Android (merge all implementations to one file).
