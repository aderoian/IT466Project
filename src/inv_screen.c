#include "simple_logger.h"

#include "gfc_vector.h"
#include "gfc_input.h"
#include "gf2d_font.h"
#include "gf2d_mouse.h"
#include "ui.h"

#include "player.h"
#include "resource.h"
#include "inv_screen.h"

typedef struct InvMenu_s {
    int numPages;
    int currPage;
    UIElement *baseUI;
    UIElement ***pages;
} InvMenu;

typedef struct InvCardData_s {
    ResourceAmount* resource;
    GFC_Vector2D pos;
} InvCardData;

static InvMenu m_invMenu = {0};

void inv_menu_update(float deltaTime);
void inv_menu_draw();
void inv_menu_click();
void inv_menu_rightClick();
void inv_menu_close();

void inv_menu_init() {
    m_invMenu.baseUI = ui_element_create_simple("images/ui/inventory/inventory_background.png", gfc_vector2d(0, 0));
    m_invMenu.baseUI->update = inv_menu_update;
    m_invMenu.baseUI->draw = inv_menu_draw;
    m_invMenu.baseUI->onClose = inv_menu_close;
}

void inv_menu_free_pages(InvMenu* menu) {
    int i, j;
    if (menu->pages) {
        for (i = 0; i < menu->numPages; i++) {
            for (j = 0; j < 3; j++) {
                ui_element_free(menu->pages[i][j]);
            }
            free(menu->pages[i]);
        }
        free(menu->pages);
        menu->pages = NULL;
    }
}

void inv_menu_create_pages(InvMenu* menu, ResourceAmount* inventory, Uint32 count) {
    int numPages, i, j, transIdx;
    InvCardData* data;
    GFC_Vector2D pos, iconPos;
    if (!menu) return;

    if (menu->pages) inv_menu_free_pages(menu); 
    if (!inventory) {
        menu->pages = NULL;
        menu->numPages = 0;
        return;
    }

    numPages = count % 3 == 0 ? count / 3 : (count / 3) + 1;
    menu->pages = (UIElement***) gfc_allocate_array(sizeof(UIElement**), numPages);
    for (i = 0; i < numPages; i++) {
        pos = gfc_vector2d(284, 225);
        menu->pages[i] = (UIElement**) gfc_allocate_array(sizeof(UIElement*), 3);
        for (j = 0; j < 3; j++) {
            transIdx = i * 3 + j;
            if (transIdx >= count) break;

            pos.x = 284 + j * 256;
            menu->pages[i][j] = ui_element_create_simple("images/ui/inventory/inventory_card.png", pos);
            data = gfc_allocate_array(sizeof(InvCardData), 1);
            data->pos = pos;
            data->resource = &inventory[transIdx];
            menu->pages[i][j]->data = data;
            
            gfc_vector2d_add(iconPos, pos, gfc_vector2d(25, 25));
            menu->pages[i][j]->children = gfc_allocate_array(sizeof(UIElement*), 1);
            menu->pages[i][j]->children[0] = ui_element_create_simple(inventory[transIdx].resource->icon, iconPos);
            menu->pages[i][j]->childCount = 1;
        }
    }

    menu->numPages = numPages;
}

void inv_menu_open() {
    if (!player) return;
    inv_menu_create_pages(&m_invMenu, ((PlayerData*)player->data)->inventory, ((PlayerData*)player->data)->invSize);
    m_invMenu.currPage = 0;

    if (!m_invMenu.pages) {
        return; // TODO: Send message that there are no trades.
    }
    ui_open_menu(m_invMenu.baseUI);
}

void inv_menu_update(float deltaTime) {
    int delta = 0;
    if (!m_invMenu.pages) return;

    if (gfc_input_command_pressed("pagenext")) delta += 1;
    if (gfc_input_command_pressed("pagelast")) delta -= 1;
    m_invMenu.currPage = (m_invMenu.currPage + delta < 0 ? 0 : m_invMenu.currPage + delta) % m_invMenu.numPages;
}

void inv_menu_draw() {
    char buffer[50];
    int i;
    UIElement *card;
    InvCardData *data;
    GFC_Vector2D pos;
    gf2d_sprite_draw_image(m_invMenu.baseUI->sprite, m_invMenu.baseUI->position);
    for (i = 0; i < 3; i++) {
        card = m_invMenu.pages[m_invMenu.currPage][i];
        if (card) {
            ui_draw_element(card);
            data = (InvCardData*) card->data;
            gfc_vector2d_add(pos, data->pos, gfc_vector2d(25, 185));
            sprintf(buffer, "Resource:");
            gf2d_font_draw_line_tag(buffer, FT_Small, GFC_COLOR_WHITE, pos);
            pos.y += 20;
            sprintf(buffer, "%s x%d", data->resource->resource->name, data->resource->amount);
            gf2d_font_draw_line_tag(buffer, FT_Small, GFC_COLOR_WHITE, pos);
        }
    }

    sprintf(buffer, "Page %d/%d", m_invMenu.currPage + 1, m_invMenu.numPages);
    gf2d_font_draw_line_tag(buffer, FT_Normal, GFC_COLOR_WHITE, gfc_vector2d(538, 558));
}

void inv_menu_close() {
    inv_menu_free_pages(&m_invMenu);
}

