#ifndef CB_TILED_H
#define CB_TILED_H

#include "2d.h"

typedef struct {
    // tile set
    cbTexture tileSet;
    int tileWidth, tileHeight;
    // tile map
    int width, height;
    int *tiles; // width * height count
    // rendering
    float x, y;
} cbTiledLayer;

cbTiledLayer* cbCreateTiledLayer(int width, int height, cbTexture tiles, int tileWidth, int tileHeight);

void cbFillTiledLayer(cbTiledLayer* tiled, int *data);
void cbFillTiledLayerFromCsv(cbTiledLayer* tiled, const char *csv);

void cbRenderTiledLayer(cbTiledLayer* tiled);

void cbDestroycbTiledLayer(cbTiledLayer* tiled);

#endif
