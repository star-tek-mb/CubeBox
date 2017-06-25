// renders images and sprites
// to render fonts see font.h
#ifndef CB_2D_H
#define CB_2D_H

#include "glad.h"

#include <stdbool.h>

// you can freely copy this texture
// but you must delete it once
// do not change anything
typedef struct {
    GLuint id;
    int width;
    int height;
    int channels;
} cbTexture;

// open image
cbTexture cbLoadTexture(const char *path);

// free image
void cbDestroyTexture(cbTexture texture);

// you can freely copy this sprite
// to multiple renders
// no need to delete it
typedef struct {
    cbTexture texture; // texture
    float u0, v0, u1, v1; // subimage
    float x, y, w, h; // position and scale
    float r, ox, oy; // rotation and its origin
    // origin must be in range (-1; 1)
} cbImage;

// create image from existing texture
cbImage cbCreateImage(cbTexture texture);

// create subimage from existing texture
cbImage cbCreateSubimage(cbTexture texture, int x, int y, int w, int h);

// animated sprite created from spritesheet
// update animatedSprite with cbUpdateAnimatedSprite each frame
// render with cbRenderSprite(animatedSprite.sprite)
// do not forget to delete it
typedef struct {
    // embed image struct
    cbTexture texture;
    float u0, v0, u1, v1;
    float x, y, w, h;
    float r, ox, oy;
    // one frame
    int frameWidth, frameHeight;
    // animation state
    bool looped, playing;
    // time
    float currentTime;
    float frameTime;
    int currentFrame;
    // frame sequences
    int *sequence;
} cbSprite;

// create animated sprite from spritesheet with given frame dimension
cbSprite* cbCreateSprite(cbTexture texture, int frameWidth, int frameHeight);

// set animated sprite frame (it is static)
// frame is a nth image (with frameWidth and frameHeight) in spritesheet
void cbSpriteSetFrame(cbSprite* sprite, int frame);

// play given animation with frame sequence (terminate with -1), with speed, and looped
void cbSpritePlayAnimation(cbSprite* sprite, const int *sequence, int framePerSecond, bool loop);
void cbSpriteStopAnimation(cbSprite* sprite); // stop animation

// updates time, frames and internally uvs of sprite
void cbSpriteUpdate(cbSprite* sprite, float delta);

// frees animation and sprite
void cbDestroySprite(cbSprite* sprite);

// init resources
void cbInit2DRenderer();
void cbDestroy2DRenderer();

// rendering
void cbStart2DRenderer();
void cbRenderImage(cbImage image);
void cbRenderSprite(cbSprite *sprite);
void cbStop2DRenderer();

#endif
