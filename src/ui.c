#include "simple_logger.h"

#include "gfc_input.h"

#include "gf2d_mouse.h"

#include "ui.h"

typedef struct {
    UIElement* overlay;
    UIElement* menu;
    int blocking;
    GFC_List *commandCallbacks;
    GFC_TextWord lastCommand;
} UIManager;

static UIManager ui_manager = {0};

void ui_init() {
    ui_manager.commandCallbacks = gfc_list_new();
    gfc_word_clear(ui_manager.lastCommand);
    atexit(ui_close_menu);
}
void ui_close() {
    int i;
    for (i = 0; i < gfc_list_count(ui_manager.commandCallbacks); i++) {
        free(gfc_list_get_nth(ui_manager.commandCallbacks, i));
    }
    gfc_list_delete(ui_manager.commandCallbacks);
}

int ui_open_menu(UIElement* e) {
    if (ui_manager.menu) return 0;

    ui_manager.menu = e;
    if (ui_manager.menu->onOpen) ui_manager.menu->onOpen();
    gf2d_mouse_show();
    ui_manager.blocking = 1;
    return 1;
}

void ui_close_menu() {
    if (!ui_manager.menu) return;
    if (ui_manager.menu->onClose) ui_manager.menu->onClose();
    ui_manager.menu = NULL;
    ui_manager.blocking = 0;
    gf2d_mouse_hide();
}

void ui_update(float deltaTime) {
    int i, j;
    UIElement* el;
    UIOpenCommandCallback* cb;
    if (ui_manager.commandCallbacks) {
        for (i = 0; i < gfc_list_count(ui_manager.commandCallbacks); i++) {
            cb = (UIOpenCommandCallback*) gfc_list_get_nth(ui_manager.commandCallbacks, i);
            if (gfc_input_command_pressed(cb->command)) {
                if (ui_manager.menu && gfc_word_cmp(ui_manager.lastCommand, cb->command) == 0) {
                    ui_close_menu();
                    gfc_word_clear(ui_manager.lastCommand);
                    break;
                }

                if (!ui_manager.menu) {
                    el = cb->commandCallback();
                    if (!el) break;

                    ui_open_menu(el);
                    gfc_word_cpy(ui_manager.lastCommand, cb->command);
                }
            }
        }
    }

    if (ui_manager.menu && gf2d_mouse_in_rect(ui_manager.menu->localBounding)) {
        if (gf2d_mouse_button_pressed(0)) {
            if (ui_manager.menu->onClick) ui_manager.menu->onClick();
        } else if (gf2d_mouse_button_pressed(2)) {
            if (ui_manager.menu->onRightClick) ui_manager.menu->onRightClick();
        } else if (ui_manager.menu->onHover) ui_manager.menu->onHover();
    }


    el = ui_manager.overlay;
    if (el) {
        if (el->update) el->update(deltaTime);
        for (j = 0; j < el->childCount; j++) {
            if (el->children[j] && el->children[j]->update) {
                el->children[j]->update(deltaTime);
            }
        }    
    }

    el = ui_manager.menu;
    if (el) {
        if (el->update) el->update(deltaTime);
        for (j = 0; j < el->childCount; j++) {
            if (el->children[j] && el->children[j]->update) {
                el->children[j]->update(deltaTime);
            }
        }
    }
}

void ui_draw() {
    ui_draw_element(ui_manager.overlay);
    ui_draw_element(ui_manager.menu);
}

void ui_draw_element(UIElement* el) {
    int i;
    if (!el) return;
    if (el->draw) el->draw();
    else {
        gf2d_sprite_draw_image(el->sprite, el->position);
        for (i = 0; i < el->childCount; i++) {
            if (el->children[i]) {
                ui_draw_element(el->children[i]);
            }
        }
    }
}

int ui_blocking() {
    return ui_manager.blocking;
}

void ui_set_overlay(UIElement* el) {
    ui_manager.overlay = el;
}

int register_command_callback(GFC_TextWord command, UIElement* (*commandCallback) ()) {
    int i;
    UIOpenCommandCallback* cb;
    if (!command || !commandCallback || !ui_manager.commandCallbacks) return 0;
    for (i = 0; i < gfc_list_count(ui_manager.commandCallbacks); i++) {
        cb = (UIOpenCommandCallback*) gfc_list_get_nth(ui_manager.commandCallbacks, i);
        if (gfc_word_cmp(command, cb->command)) return 0;
    }

    cb = gfc_allocate_array(sizeof(UIOpenCommandCallback), 1);
    gfc_word_cpy(cb->command, command);
    cb->commandCallback = commandCallback;
    gfc_list_append(ui_manager.commandCallbacks, cb);
    return 1;
}

UIElement* ui_element_create_simple(const char *filename, GFC_Vector2D pos) {
    Sprite* sprite;
    UIElement* el;
    if (!filename) return NULL;
    sprite = gf2d_sprite_load_image(filename);
    if (!sprite) return NULL;
    el = gfc_allocate_array(sizeof(UIElement), 1);
    if (!el) {
        gf2d_sprite_free(sprite);
        return NULL;
    }

    el->sprite = sprite;
    el->position = pos;
    el->children = NULL;
    el->childCount = 0;
    gfc_rect_set(el->localBounding, pos.x, pos.y, (sprite->frameWidth * sprite->widthPercent), (sprite->frameHeight * sprite->heightPercent));
    return el;
}
