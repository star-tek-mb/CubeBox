#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

cbShader cbCreateShader(const char *vsrc, const char *fsrc) {
    cbShader program;
    GLchar infoLog[512];
    GLint success;
    GLuint vsh, fsh;

    vsh = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vsh, 1, &vsrc, 0);
    glCompileShader(vsh);
    glGetShaderiv(vsh, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vsh, 512, NULL, infoLog);
        printf("vertex shader compilation error:\n%s\n", infoLog);
    }

    fsh = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fsh, 1, &fsrc, 0);
    glCompileShader(fsh);
    glGetShaderiv(fsh, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fsh, 512, NULL, infoLog);
        printf("fragment shader compilation error:\n%s\n", infoLog);
    }

    program = glCreateProgram();
    glAttachShader(program, vsh);
    glAttachShader(program, fsh);
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        printf("shader program linking error:\n%s\n", infoLog);
    }

    glDeleteShader(vsh);
    glDeleteShader(fsh);
    return program;
}

void cbDeleteShader(cbShader shader) {
    glDeleteProgram(shader);
}

cbList* cbNewList(int elementSize) {
    cbList* list = malloc(sizeof(cbList));
    list->length = 0;
    list->elementSize = elementSize;
    list->begin = NULL;
    return list;
}

void* cbListGet(cbList* list, int at) {
    assert(at >= 0 && at < list->length);

    int i = 0;
    cbListElement* iter = list->begin;
    while (iter != NULL) {
        if (i == at) {
            return iter->data;
        }
        i++;
        iter = iter->next;
    }
    return NULL;
}

void cbListInsert(cbList* list, void* data, int at) {
    assert(at >= 0 && at <= list->length);

    // initialize element and increlement list length
    cbListElement* element = malloc(sizeof(cbListElement));
    element->data = malloc(list->elementSize);
    memcpy(element->data, data, list->elementSize);
    list->length++;

    // find element before and after
    cbListElement *before = NULL, *after = NULL;
    cbListElement* iter = list->begin;
    int i = 0;
    while (iter != NULL) {
        if (i == at - 1) {
            before = iter;
        }
        if (i == at) {
            after = iter;
            break;
        }
        i++;
        iter = iter->next;
    }

    // insert at begin
    if (before == NULL) {
        element->next = list->begin;
        list->begin = element;
        return;
    }

    // insert at middle and end
    before->next = element;
    element->next = after;
}

void cbListAppend(cbList* list, void* data) {
    cbListInsert(list, data, list->length);
}

void cbListPrepend(cbList* list, void* data) {
    cbListInsert(list, data, 0);
}

void cbListRemove(cbList* list, int at) {
    assert(at >= 0 && at < list->length);
    // decrement list length
    list->length--;

    // find element before, after and element to remove
    cbListElement *before = NULL, *after = NULL, *element = NULL;
    cbListElement* iter = list->begin;
    int i = 0;
    while (iter != NULL) {
        if (i == at - 1) {
            before = iter;
        }
        if (i == at) {
            element = iter;
        }
        if (i == at + 1) {
            after = iter;
            break;
        }
        i++;
        iter = iter->next;
    }

    // remove whole list
    if (before == NULL && after == NULL) {
        free(element->data);
        free(element);
        list->begin = NULL;
        return;
    }

    // remove at begin
    if (before == NULL) {
        free(element->data);
        free(element);
        list->begin = after;
        return;
    }

    // remove at middle and end
    free(element->data);
    free(element);
    before->next = after;
}

void cbFreeList(cbList* list) {
    cbListElement* iter = list->begin;
    while (iter != NULL) {
        cbListElement* tofree = iter;
        iter = iter->next;

        free(tofree->data);
        free(tofree);
    }
    free(list);
}
