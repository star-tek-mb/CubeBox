#include "tiled.h"
#include "2d.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

cbTiledLayer* cbCreateTiledLayer(int width, int height, cbTexture tiles, int tileWidth, int tileHeight) {
    cbTiledLayer* tiled = malloc(sizeof(cbTiledLayer));
    tiled->width = width;
    tiled->height = height;
    tiled->tileSet = tiles;
    tiled->tileWidth = tileWidth;
    tiled->tileHeight = tileHeight;
    tiled->tiles = malloc(width * height * sizeof(int));

    tiled->x = tiled->y = 0.0f;
    return tiled;
}

void cbFillTiledLayer(cbTiledLayer* tiled, int *data) {
    memcpy(tiled->tiles, data, tiled->tileWidth * tiled->tileHeight * sizeof(int));
}

void cbFillTiledLayerFromCsv(cbTiledLayer* tiled, const char *path) {
    FILE *csv = fopen(path, "r");
    int s, i = 0;
    while (fscanf(csv, "%d,", &s) == 1) {
        tiled->tiles[i++] = s;
    }
    fclose(csv);
}

void cbRenderTiledLayer(cbTiledLayer* tiled) {
    for (int i = 0; i < tiled->width; i++) {
        for (int j = 0; j < tiled->height; j++) {
            int tile = tiled->tiles[i + j * tiled->width];

            int row = tile % (tiled->tileSet.width / tiled->tileWidth);
            int col = tile / (tiled->tileSet.width / tiled->tileWidth);

            cbImage tileImage = cbCreateSubimage(tiled->tileSet, row * tiled->tileWidth, col * tiled->tileHeight, tiled->tileWidth, tiled->tileHeight);
            tileImage.x = i * tiled->tileWidth;
            tileImage.y = j * tiled->tileHeight;
            cbRenderImage(tileImage);
        }
    }
}

void cbDestroyTiledLayer(cbTiledLayer* tiled) {
    free(tiled->tiles);
    free(tiled);
}
