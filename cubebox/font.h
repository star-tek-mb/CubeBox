#ifndef CB_FONT_H
#define CB_FONT_H

#include "linmath.h"

#ifndef CB_FONT_CACHE_SIZE
    #define CB_FONT_CACHE_SIZE 512
#endif

typedef int cbFont; // font descriptor

// load/destroy fonts
cbFont cbLoadFont(const char *path, int size);
void cbDestroyFont(cbFont id);

// init/destroy font renderer
void cbInitFontRenderer();
void cbDestroyFontRenderer();

// start/stop font renderer
void cbStartFontRenderer();
void cbStopFontRenderer();

// renders text with font id in x,y with color
// align is left | baseline
void cbRenderText(cbFont id, const char *text, float x, float y, vec3 color);

int cbTextWidth(cbFont id, const char *text);
int cbFontHeight(cbFont id); // ascender - descender + lineGap

#endif
