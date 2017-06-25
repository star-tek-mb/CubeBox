#include "font.h"
#include "engine.h"
#include "utils.h"
#include "glad.h"
#include "linmath.h"
#include "stretchy_buffer.h"
#include "stb_rect_pack.h"
#include "stb_truetype.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <inttypes.h>

// UTF-8 decoder
// Copyright (c) 2008-2009 Bjoern Hoehrmann <bjoern@hoehrmann.de>
// See http://bjoern.hoehrmann.de/utf-8/decoder/dfa/ for details.

#define UTF8_ACCEPT 0
#define UTF8_REJECT 1

static const uint8_t utf8d[] = {
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 00..1f
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 20..3f
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 40..5f
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 60..7f
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9, // 80..9f
  7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7, // a0..bf
  8,8,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, // c0..df
  0xa,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x4,0x3,0x3, // e0..ef
  0xb,0x6,0x6,0x6,0x5,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8, // f0..ff
  0x0,0x1,0x2,0x3,0x5,0x8,0x7,0x1,0x1,0x1,0x4,0x6,0x1,0x1,0x1,0x1, // s0..s0
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,0,1,0,1,1,1,1,1,1, // s1..s2
  1,2,1,1,1,1,1,2,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1, // s3..s4
  1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,3,1,3,1,1,1,1,1,1, // s5..s6
  1,3,1,1,1,1,1,3,1,3,1,1,1,1,1,1,1,3,1,1,1,1,1,1,1,1,1,1,1,1,1,1, // s7..s8
};

static uint32_t decode(uint32_t* state, uint32_t* codep, uint32_t byte) {
  uint32_t type = utf8d[byte];

  *codep = (*state != UTF8_ACCEPT) ?
    (byte & 0x3fu) | (*codep << 6) :
    (0xff >> type) & (byte);

  *state = utf8d[256 + *state*16 + type];
  return *state;
}
// UTF-8 decoder

// texture with FONT_CACHE_SIZE width and height
// modifies gl texture itself
struct cbFontTexture {
    GLuint id;
    // glyph packer
    stbrp_context *packer;
    stbrp_node *nodes; // CB_FONT_CACHE_SIZE size
};

struct cbFontGlyph {
    // unicode character
    unsigned int codepoint;
    // stored in texture
    struct cbFontTexture texture; // copy
    int x0, y0, x1, y1;
    float advance, xOffset, yOffset;
};

struct cbFontImpl {
    cbFont id;
    int size;

    stbtt_fontinfo stbFont;
    unsigned char *fontData;

    struct cbFontGlyph *glyphs;  // stretchy buffer
    struct cbFontTexture *textures; // stretchy buffer
};

static unsigned char *emptyData = NULL; // black image
static struct cbFontImpl *fonts = NULL; // stretchy buffer

cbFont cbLoadFont(const char *path, int size) {
    struct cbFontImpl font;
    font.size = size;
    font.glyphs = NULL;
    font.textures = NULL;
    font.id = sb_count(fonts);

    // read font file
    FILE *fontFile = fopen(path, "r");
    fseek(fontFile, 0, SEEK_END);
    int fontDataSize = (int) ftell(fontFile);
    fseek(fontFile, 0, SEEK_SET);
    font.fontData = malloc(fontDataSize);
    fread(font.fontData, 1, fontDataSize, fontFile);
    fclose(fontFile);

    stbtt_InitFont(&font.stbFont, font.fontData, 0);
    sb_push(fonts, font);

    return font.id;
}

void cbDestroyFont(cbFont id) {
    if (id == -1) return; // was deleted
    struct cbFontImpl* font = &fonts[id];
    if (font->id == -1) return; // second check

    free(font->fontData);
    for (int i = 0; i < sb_count(font->textures); i++) {
        glDeleteTextures(1, &font->textures[i].id);
        free(font->textures[i].packer);
        free(font->textures[i].nodes);
    }

    sb_free(font->textures);
    sb_free(font->glyphs);
    font->id = -1; // mark as deleted
}

static struct cbFontTexture* createFontTexture(struct cbFontImpl* font) {
    struct cbFontTexture fontTexture;

    // create texture
    glGenTextures(1, &fontTexture.id);
    glBindTexture(GL_TEXTURE_2D, fontTexture.id);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, CB_FONT_CACHE_SIZE, CB_FONT_CACHE_SIZE, 0, GL_RED, GL_UNSIGNED_BYTE, emptyData);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // init glyph packer
    fontTexture.packer = malloc(sizeof(stbrp_context));
    fontTexture.nodes = malloc(sizeof(stbrp_node) * CB_FONT_CACHE_SIZE);
    stbrp_init_target(fontTexture.packer, CB_FONT_CACHE_SIZE, CB_FONT_CACHE_SIZE, fontTexture.nodes, CB_FONT_CACHE_SIZE);

    sb_push(font->textures, fontTexture);
    return &font->textures[sb_count(font->textures) - 1]; // return pointer
}

static struct cbFontGlyph* loadGlyph(struct cbFontImpl* font, unsigned int codepoint) {
    // find glyph if exists
    for (int i = 0; i < sb_count(font->glyphs); i++) {
        if (font->glyphs[i].codepoint == codepoint) {
            return &font->glyphs[i];
        }
    }

    // get metrics
    int gIndex, advance, lsb, x0, y0, x1, y1, gw, gh;
    float scale;
    scale = stbtt_ScaleForPixelHeight(&font->stbFont, font->size);
    gIndex = stbtt_FindGlyphIndex(&font->stbFont, codepoint);

    // glyph not found
    if (gIndex == 0) return NULL;

    stbtt_GetGlyphHMetrics(&font->stbFont, gIndex, &advance, &lsb);
	stbtt_GetGlyphBitmapBox(&font->stbFont, gIndex, scale, scale, &x0, &y0, &x1, &y1);
	gw = x1 - x0;
    gh = y1 - y0;

    // get last texture or create new one
    struct cbFontTexture* texture = NULL;
    if (sb_count(font->textures) == 0) {
        texture = createFontTexture(font);
    } else {
        texture = &font->textures[sb_count(font->textures) - 1];
    }

    // try to pack
    stbrp_rect rect;
    rect.w = gw;
    rect.h = gh;
    if (stbrp_pack_rects(texture->packer, &rect, 1) == 0) {
        // create new texture and pack to it
        texture = createFontTexture(font);
        stbrp_pack_rects(texture->packer, &rect, 1);
    }

    // push glyph to font index
    struct cbFontGlyph glyph;
    glyph.codepoint = codepoint;
    glyph.texture = *texture;
    glyph.x0 = rect.x;
    glyph.y0 = rect.y;
    glyph.x1 = glyph.x0 + gw;
    glyph.y1 = glyph.y0 + gh;
    glyph.advance = scale * advance;
    glyph.xOffset = (float) x0;
    glyph.yOffset = (float) y0;
    sb_push(font->glyphs, glyph);

    // modify texture
    unsigned char *bitmap = malloc(gw * gh);
    stbtt_MakeGlyphBitmap(&font->stbFont, bitmap, gw, gh, gw, scale, scale, gIndex);
	glBindTexture(GL_TEXTURE_2D, texture->id);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexSubImage2D(GL_TEXTURE_2D, 0, glyph.x0, glyph.y0, gw, gh, GL_RED, GL_UNSIGNED_BYTE, bitmap);
    free(bitmap);

    return &font->glyphs[sb_count(font->glyphs) - 1];
}

struct quad {
    float x0, y0, x1, y1;
    float u0, v0, u1, v1;
};

static void getQuad(struct cbFontGlyph* glyph, float *x, float *y, struct quad *quad) {
    int rx = floorf(*x + glyph->xOffset);
	int ry = floorf(*y + glyph->yOffset);
	
	quad->x0 = (float) rx;
	quad->y0 = (float) ry;
	quad->x1 = (float) rx + (glyph->x1 - glyph->x0);
	quad->y1 = (float) ry + (glyph->y1 - glyph->y0);
	
	quad->u0 = (float) (glyph->x0) / CB_FONT_CACHE_SIZE;
	quad->v0 = (float) (glyph->y0) / CB_FONT_CACHE_SIZE;
	quad->u1 = (float) (glyph->x1) / CB_FONT_CACHE_SIZE;
    quad->v1 = (float) (glyph->y1) / CB_FONT_CACHE_SIZE;

    *x += glyph->advance;
}

// resources to init
static GLuint shaderProgram = 0;
static GLuint vbo, vao, currentTexture;
static float *vertices = NULL; // stretchy buffer
static mat4x4 projection;
static const char *vertexShader = "#version 330 core                        \n"
            "in vec4 vertex;                                                \n"
            "out vec2 texcoords;                                            \n"
            "void main() {                                                  \n"
            "gl_Position = vec4(vertex.xy, 0.0, 1.0);                       \n"
            "texcoords = vertex.zw;                                         \n"
            "}                                                              \n";

static const char *fragmentShader = "#version 330 core                      \n"
            "in vec2 texcoords;                                             \n"
            "out vec4 color;                                                \n"
            "uniform sampler2D text;                                        \n"
            "uniform vec3 textColor;                                        \n"
            "void main() {                                                  \n"
            "vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, texcoords).r);\n"
            "color = vec4(textColor.rgb, 1.0) * sampled;                    \n"
            "}                                                              \n";

void cbInitFontRenderer() {
    shaderProgram = cbCreateShader(vertexShader, fragmentShader);
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    emptyData = malloc(CB_FONT_CACHE_SIZE * CB_FONT_CACHE_SIZE);
    memset(emptyData, 0, CB_FONT_CACHE_SIZE * CB_FONT_CACHE_SIZE);
}

void cbDestroyFontRenderer() {
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    cbDeleteShader(shaderProgram);

    free(emptyData);
    for (int i = 0; i < sb_count(fonts); i++) {
        cbDestroyFont(fonts[i].id);
    }
    sb_free(fonts);
    fonts = NULL;
}

void cbStartFontRenderer() {
    // get ortho from window size
    int viewX, viewY;
    cbGetSize(&viewX, &viewY);
    mat4x4_ortho(projection, 0.0f, (float) viewX, (float) viewY, 0.0f, -1.0f, 1.0f);

    // setup gl
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // setup shader
    glUseProgram(shaderProgram);
    glUniform1i(glGetUniformLocation(shaderProgram, "text"), 0);
}

static void flushRenderer(GLuint texture) {
    if (sb_count(vertices) == 0)
        return;

    // bind vertices
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sb_count(vertices) * sizeof(GLfloat), vertices, GL_STATIC_DRAW);

    // bind texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    // draw sprite
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
    glDrawArrays(GL_TRIANGLES, 0, sb_count(vertices) / 4);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // clear vertices
    sb_free(vertices);
    vertices = NULL;
}

void cbRenderText(cbFont id, const char *text, float x, float y, vec3 color) {
    glUniform3fv(glGetUniformLocation(shaderProgram, "textColor"), 1, color);

    struct cbFontImpl* font = &fonts[id];

    uint32_t state = 0, codepoint;
    struct quad quad;
    currentTexture = 0;

    while (*text) {
        if (decode(&state, &codepoint, *(uint8_t*) text++)) {
            continue; // not unicode char yet
        }

        struct cbFontGlyph* glyph = loadGlyph(font, codepoint);
        if (glyph == NULL) {
            continue; // glyph not found
        }

        // batching
        if (currentTexture != glyph->texture.id) {
            flushRenderer(currentTexture);
        }
        currentTexture = glyph->texture.id;

        // push to vertices buffer
        getQuad(glyph, &x, &y, &quad);
        vec4 q0, q1;
        // multiply by projection
        mat4x4_mul_vec4(q0, projection, (vec4) {quad.x0, quad.y0, 0.0, 1.0});
        mat4x4_mul_vec4(q1, projection, (vec4) {quad.x1, quad.y1, 0.0, 1.0});
        sb_push(vertices, q0[0]); sb_push(vertices, q0[1]); sb_push(vertices, quad.u0); sb_push(vertices, quad.v0);
        sb_push(vertices, q1[0]); sb_push(vertices, q0[1]); sb_push(vertices, quad.u1); sb_push(vertices, quad.v0);
        sb_push(vertices, q0[0]); sb_push(vertices, q1[1]); sb_push(vertices, quad.u0); sb_push(vertices, quad.v1);
        sb_push(vertices, q1[0]); sb_push(vertices, q1[1]); sb_push(vertices, quad.u1); sb_push(vertices, quad.v1);
        sb_push(vertices, q1[0]); sb_push(vertices, q0[1]); sb_push(vertices, quad.u1); sb_push(vertices, quad.v0);
        sb_push(vertices, q0[0]); sb_push(vertices, q1[1]); sb_push(vertices, quad.u0); sb_push(vertices, quad.v1);
    }

    flushRenderer(currentTexture);
}

int cbTextWidth(cbFont id, const char *text) {
    struct cbFontImpl* font = &fonts[id];
    float x = 0, y = 0;
    uint32_t state = 0, codepoint;
    struct quad quad;

    while (*text) {
        if (decode(&state, &codepoint, *(uint8_t*) text++)) {
            continue; // not unicode char yet
        }

        struct cbFontGlyph* glyph = loadGlyph(font, codepoint);
        if (glyph == NULL) {
            continue; // glyph not found
        }
        getQuad(glyph, &x, &y, &quad);
    }
    return (int) x;
}

int cbFontHeight(cbFont id) {
    struct cbFontImpl* font = &fonts[id];
    int ascender, descender, gap;
    stbtt_GetFontVMetrics(&font->stbFont, &ascender, &descender, &gap);
    return font->size + gap; // font size calculated with stbtt_ScaleForPixelHeight
}

void cbStopFontRenderer() {
    glUseProgram(0);
}

