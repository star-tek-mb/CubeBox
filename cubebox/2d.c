#include "2d.h"
#include "engine.h"
#include "utils.h"
#include "stb_image.h"
#include "linmath.h"
#include "stretchy_buffer.h"

#include <stdbool.h>

cbTexture cbLoadTexture(const char *path) {
    cbTexture texture;
    unsigned char *data = stbi_load(path, &texture.width, &texture.height, &texture.channels, 0);

    GLuint format = 0;
    switch (texture.channels) {
    case 1:
        format = GL_RED;
        break;
    case 2:
        format = GL_RG;
        break;
    case 3:
        format = GL_RGB;
        break;
    case 4:
        format = GL_RGBA;
        break;
    }

    // create texture with default params
    glGenTextures(1, &texture.id);
    glBindTexture(GL_TEXTURE_2D, texture.id);
    glTexImage2D(GL_TEXTURE_2D, 0, format, texture.width, texture.height, 0, format, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);

    free(data);
    return texture;
}

void cbDestroyTexture(cbTexture texture) {
    glDeleteTextures(1, &texture.id);
}

cbImage cbCreateImage(cbTexture texture) {
    return cbCreateSubimage(texture, 0, 0, texture.width, texture.height);
}

cbImage cbCreateSubimage(cbTexture texture, int x, int y, int w, int h) {
    cbImage image;
    image.texture = texture;
    image.u0 = (float) x / texture.width;
	image.v0 = (float) y / texture.height;
	image.u1 = (float) (x + w) / texture.width;
	image.v1 = (float) (y + h) / texture.height;

    image.x = 0.0f;
    image.y = 0.0f;
    image.w = (float) w;
    image.h = (float) h;
    image.r = 0.0f;
    // default origin - center
    image.ox = 0.5f;
    image.oy = 0.5f;
    return image;
}

cbSprite* cbCreateSprite(cbTexture texture, int frameWidth, int frameHeight) {
    cbSprite* sprite = malloc(sizeof(cbSprite));
    // init animation params
    sprite->sequence = NULL;
    sprite->currentTime = 0.0f;
    sprite->currentFrame = 0;
    sprite->frameWidth = frameWidth;
    sprite->frameHeight = frameHeight;
    sprite->playing = false;
    // init render image
    sprite->texture = texture;
    sprite->x = 0.0f;
    sprite->y = 0.0f;
    sprite->w = frameWidth;
    sprite->h = frameHeight;
    sprite->r = 0.0f;
    // default origin - center
    sprite->ox = 0.5f;
    sprite->oy = 0.5f;
    // set first frame as render image
    cbSpriteSetFrame(sprite, 0);
    return sprite;
}

void cbSpriteSetFrame(cbSprite* sprite, int frame) {
    // get row, col
    int i = frame % (sprite->texture.width / sprite->frameWidth);
    int j = frame / (sprite->texture.width / sprite->frameWidth);
    // set uvs
    sprite->u0 = (float) (i * sprite->frameWidth) / sprite->texture.width;
	sprite->v0 = (float) (j * sprite->frameHeight) / sprite->texture.height;
	sprite->u1 = (float) ((i + 1) * sprite->frameWidth) / sprite->texture.width;
	sprite->v1 = (float) ((j + 1) * sprite->frameHeight) / sprite->texture.height;
}

void cbSpritePlayAnimation(cbSprite* sprite, const int *sequence, int framePerSecond, bool loop) {
    if (sprite->playing) return; // already playing

    // setup parameters
    sprite->playing = true;
    sprite->currentTime = 0.0f;
    sprite->looped = loop;
    sprite->frameTime = (float) 1 / framePerSecond;
    sprite->currentFrame = 0;

    // copy animation sequence
    int frame;
    while ((frame = *sequence++) != -1) {
        sb_push(sprite->sequence, frame);
    }
}

void cbSpriteStopAnimation(cbSprite* sprite) {
    if (!sprite->playing) return; // already stopped

    // free animation
    sprite->playing = false;
    sb_free(sprite->sequence);
    sprite->sequence = NULL;
}

void cbSpriteUpdate(cbSprite* sprite, float delta) {
    if (!sprite->playing) return;
    sprite->currentTime += delta;

    if (sprite->currentTime > sprite->frameTime) {
        sprite->currentTime -= sprite->frameTime;

        if (sprite->currentFrame + 1 < sb_count(sprite->sequence)) {
            sprite->currentFrame++;
        } else {
            sprite->currentFrame = 0;

            if (!sprite->looped) {
                cbSpriteStopAnimation(sprite);
                return;
            }
        }

        cbSpriteSetFrame(sprite, sprite->sequence[sprite->currentFrame]);
    }
}

void cbDestroySprite(cbSprite* sprite) {
    cbSpriteStopAnimation(sprite); // clear animation sequence
    free(sprite);
}

// resources to init
static GLuint shaderProgram = 0;
static GLuint vao, vbo, currentTexture = 0;
static float *vertices = NULL; // stretchy buffer
static mat4x4 projection;
static const char *vertexShader = "#version 330 core              \n"
            "in vec4 vertex;                                      \n"
            "out vec2 texcoords;                                  \n"
            "void main() {                                        \n"
            "gl_Position = vec4(vertex.xy, 0.0, 1.0);             \n"
            "texcoords = vertex.zw;                               \n"
            "}                                                    \n";

static const char *fragmentShader = "#version 330 core            \n"
            "in vec2 texcoords;                                   \n"
            "out vec4 color;                                      \n"
            "uniform sampler2D image;                             \n"
            "void main() {                                        \n"
            "color = texture(image, texcoords);                   \n"
            "}                                                    \n";

void cbInit2DRenderer() {
    shaderProgram = cbCreateShader(vertexShader, fragmentShader);
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
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

void cbStart2DRenderer() {
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
    glUniform1i(glGetUniformLocation(shaderProgram, "image"), 0);
}

void cbRenderImage(cbImage image) {
    // batching
    if (currentTexture != image.texture.id) {
        flushRenderer(currentTexture);
    }
    currentTexture = image.texture.id;

    mat4x4 final;
    mat4x4_identity(final);
    mat4x4_mul(final, final, projection);
    // translate to position, align to origin
    mat4x4_translate_in_place(final, image.x + (image.ox * image.w), image.y + (image.oy * image.h), 0.0f);
    // rotate
    mat4x4_rotate_Z(final, final, image.r);
    // align to origin
    mat4x4_translate_in_place(final, -image.ox * image.w, -image.oy * image.h, 0.0f);
    // scale
    mat4x4_scale_aniso(final, final, image.w, image.h, 1.0f);

    // cpu transform computing
    vec4 v[4];
    mat4x4_mul_vec4(v[0], final, (vec4) {0.0, 0.0, 0.0, 1.0});
    mat4x4_mul_vec4(v[1], final, (vec4) {1.0, 0.0, 0.0, 1.0});
    mat4x4_mul_vec4(v[2], final, (vec4) {0.0, 1.0, 0.0, 1.0});
    mat4x4_mul_vec4(v[3], final, (vec4) {1.0, 1.0, 0.0, 1.0});

    // push to vertices buffer
    sb_push(vertices, v[0][0]); sb_push(vertices, v[0][1]); sb_push(vertices, image.u0); sb_push(vertices, image.v0);
    sb_push(vertices, v[1][0]); sb_push(vertices, v[1][1]); sb_push(vertices, image.u1); sb_push(vertices, image.v0);
    sb_push(vertices, v[2][0]); sb_push(vertices, v[2][1]); sb_push(vertices, image.u0); sb_push(vertices, image.v1);
    sb_push(vertices, v[3][0]); sb_push(vertices, v[3][1]); sb_push(vertices, image.u1); sb_push(vertices, image.v1);
    sb_push(vertices, v[1][0]); sb_push(vertices, v[1][1]); sb_push(vertices, image.u1); sb_push(vertices, image.v0);
    sb_push(vertices, v[2][0]); sb_push(vertices, v[2][1]); sb_push(vertices, image.u0); sb_push(vertices, image.v1);
}

void cbRenderSprite(cbSprite* sprite) {
    cbImage image;
    image.x = sprite->x;
    image.y = sprite->y;
    image.w = sprite->w;
    image.h = sprite->h;
    image.r = sprite->r;
    image.ox = sprite->ox;
    image.oy = sprite->oy;
    image.u0 = sprite->u0;
    image.v0 = sprite->v0;
    image.u1 = sprite->u1;
    image.v1 = sprite->v1;
    image.texture = sprite->texture;
    cbRenderImage(image);
}

void cbStop2DRenderer() {
    flushRenderer(currentTexture);
    glUseProgram(0);
}

void cbDestroy2DRenderer() {
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    cbDeleteShader(shaderProgram);
}
