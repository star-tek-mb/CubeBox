#ifndef CB_UTILS_H
#define CB_UTILS_H

#include "glad.h"

/// SHADERS

typedef GLuint cbShader;

cbShader cbCreateShader(const char *vsrc, const char *fsrc);
void cbDeleteShader(cbShader shader);

/// LIST

typedef struct cbListElement {
    void *data;
    struct cbListElement* next;
} cbListElement;

typedef struct {
    int length;
    int elementSize;
    cbListElement* begin;
} cbList;

// define list with size of element
cbList* cbNewList(int elementSize);

// short code for typed list
#define cbNewListFor(type) cbNewList(sizeof(type))

// get element data at position
void* cbListGet(cbList* list, int at);

// insert before element at position
// if at == 0 - prepending, if at == list->length - appending
void cbListInsert(cbList* list, void* data, int at);

// short code of insert
void cbListAppend(cbList* list, void* data);

// short code of insert
void cbListPrepend(cbList* list, void* data);

// remove element at position
void cbListRemove(cbList* list, int at);

// frees list and all its elements
void cbFreeList(cbList* list);

#endif
