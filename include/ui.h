#ifndef __UI_H__
#define __UI_H__

#include "gfc_shape.h"
#include "gf2d_sprite.h"
#include "gfc_text.h"

typedef struct UIElement_s {
    Sprite* sprite;
    GFC_Vector2D position;
    GFC_Rect localBounding;
    struct UIElement_s** children;
    int childCount;

    void (*update) (float deltaTime);
    void (*onOpen) (void);
    void (*onClose) (void);
    void (*onHover) (void);
    void (*onClick) (void);
    void (*onRightClick ) (void);
    void (*draw) (void);

    void *data;
} UIElement;

typedef struct UIOpenCommandCallback_s {
    GFC_TextWord command;
    UIElement* (*commandCallback) ();
} UIOpenCommandCallback;

void ui_init();
void ui_close();


int ui_open_menu(UIElement* e);
void ui_close_menu();

void ui_update(float deltaTime);
void ui_draw();
void ui_draw_element(UIElement* el);

int ui_blocking();

void ui_set_overlay(UIElement* el);

int register_command_callback(GFC_TextWord command, UIElement* (*commandCallback) ());

UIElement* ui_element_create_simple(const char *filename, GFC_Vector2D pos);

#endif // __UI_H__
