#include "simple_logger.h"

#include "gfc_vector.h"
#include "gfc_input.h"
#include "gf2d_font.h"
#include "gf2d_mouse.h"
#include "ui.h"

#include "player.h"
#include "civilization.h"
#include "mission_menu.h"

typedef struct MissionMenu_s {
    int numPages;
    int currPage;
    UIElement *baseUI;
    UIElement ***pages;
} MissionMenu;

typedef struct MissionCardData_s {
    CivilMission* mission;
    GFC_Vector2D pos;
} MissionCardData;

static MissionMenu m_missionMenu = {0};

void mission_menu_update(float deltaTime);
void mission_menu_draw();
void mission_menu_click();
void mission_menu_rightClick();
void mission_menu_close();

void mission_menu_init() {
    m_missionMenu.baseUI = ui_element_create_simple("images/ui/mission/mission_background.png", gfc_vector2d(0, 0));
    m_missionMenu.baseUI->update = mission_menu_update;
    m_missionMenu.baseUI->draw = mission_menu_draw;
    m_missionMenu.baseUI->onClick = mission_menu_click;
    m_missionMenu.baseUI->onRightClick = mission_menu_rightClick;
    m_missionMenu.baseUI->onClose = mission_menu_close;
}

void mission_menu_create_pages(MissionMenu* menu, GFC_List* items) {
    int numTrans, numPages, i, j, transIdx;
    CivilMission* mission;
    MissionCardData* data;
    GFC_Vector2D pos, iconPos;
    if (!items) {
        slog ("civ has no trade list");
        menu->pages = NULL;
        menu->numPages = 0;
        return;
    }

    numTrans = gfc_list_count(items);
    numPages = numTrans % 3 == 0 ? numTrans / 3 : (numTrans / 3) + 1;
    menu->pages = (UIElement***) gfc_allocate_array(sizeof(UIElement**), numPages);
    for (i = 0; i < numPages; i++) {
        pos = gfc_vector2d(284, 225);
        menu->pages[i] = (UIElement**) gfc_allocate_array(sizeof(UIElement*), 3);
        for (j = 0; j < 3; j++) {
            transIdx = i * 3 + j;
            if (transIdx >= numTrans) break;
            mission = gfc_list_get_nth(items, transIdx);

            pos.x = 284 + j * 256;
            menu->pages[i][j] = ui_element_create_simple("images/ui/mission/mission_card.png", pos);
            data = gfc_allocate_array(sizeof(MissionCardData), 1);
            data->pos = pos;
            data->mission = mission;
            menu->pages[i][j]->data = data;
            
            gfc_vector2d_add(iconPos, pos, gfc_vector2d(25, 25));
            menu->pages[i][j]->children = gfc_allocate_array(sizeof(UIElement*), 1);
            menu->pages[i][j]->children[0] = ui_element_create_simple(mission->trans->take.resource->icon, iconPos);
            menu->pages[i][j]->childCount = 1;
        }
    }

    menu->numPages = numPages;
}

void mission_menu_open() {
    if (!player) return;
    mission_menu_create_pages(&m_missionMenu, ((PlayerData*)player->data)->civilMissions);
    m_missionMenu.currPage = 0;

    if (!m_missionMenu.pages) {
        return; // TODO: Send message that there are no trades.
    }
    ui_open_menu(m_missionMenu.baseUI);
}

void mission_menu_update(float deltaTime) {
    int delta = 0;
    if (!m_missionMenu.pages) return;

    if (gfc_input_command_pressed("pagenext")) delta += 1;
    if (gfc_input_command_pressed("pagelast")) delta -= 1;
    m_missionMenu.currPage = (m_missionMenu.currPage + delta < 0 ? 0 : m_missionMenu.currPage + delta) % m_missionMenu.numPages;
}

void mission_menu_draw() {
    char buffer[50];
    int i;
    UIElement *tradeCard;
    MissionCardData *data;
    GFC_Vector2D pos;
    gf2d_sprite_draw_image(m_missionMenu.baseUI->sprite, m_missionMenu.baseUI->position);
    for (i = 0; i < 3; i++) {
        tradeCard = m_missionMenu.pages[m_missionMenu.currPage][i];
        if (tradeCard) {
            ui_draw_element(tradeCard);
            data = (MissionCardData*) tradeCard->data;
            gfc_vector2d_add(pos, data->pos, gfc_vector2d(25, 185));
            sprintf(buffer, "Collect:");
            gf2d_font_draw_line_tag(buffer, FT_Small, GFC_COLOR_WHITE, pos);
            pos.y += 20;
            sprintf(buffer, "%s x%d", data->mission->trans->take.resource->name, data->mission->trans->take.amount);
            gf2d_font_draw_line_tag(buffer, FT_Small, GFC_COLOR_WHITE, pos);

            pos.y += 30;
            sprintf(buffer, "Reward:");
            gf2d_font_draw_line_tag(buffer, FT_Small, GFC_COLOR_WHITE, pos);
            pos.y += 20;
            sprintf(buffer, "%s x%d", data->mission->trans->give.resource->name, data->mission->trans->give.amount);
            gf2d_font_draw_line_tag(buffer, FT_Small, GFC_COLOR_WHITE, pos);
        }
    }

    sprintf(buffer, "Page %d/%d", m_missionMenu.currPage + 1, m_missionMenu.numPages);
    gf2d_font_draw_line_tag(buffer, FT_Normal, GFC_COLOR_WHITE, gfc_vector2d(538, 558));
}

void mission_menu_click() {
    int i;
    UIElement* card;
    if (m_missionMenu.numPages == 0 || !m_missionMenu.pages || !m_missionMenu.pages[m_missionMenu.currPage]) return;
    for (i = 0; i < 3; i++) {
        card = m_missionMenu.pages[m_missionMenu.currPage][i];
        if (!card) break;

        if (player && gf2d_mouse_in_rect(card->localBounding)) {
            player_try_end_mission(player, ((MissionCardData*)card->data)->mission, 0);
            ui_close_menu();
            return;
        }
    }
}

void mission_menu_rightClick() {
    int i;
    UIElement* card;
    if (m_missionMenu.numPages == 0 || !m_missionMenu.pages || !m_missionMenu.pages[m_missionMenu.currPage]) return;
    for (i = 0; i < 3; i++) {
        card = m_missionMenu.pages[m_missionMenu.currPage][i];
        if (!card) break;

        if (player && gf2d_mouse_in_rect(card->localBounding)) {
            player_try_end_mission(player, ((MissionCardData*)card->data)->mission, 1);
            ui_close_menu();
            return;
        }
    }
}

void mission_menu_close() {
    int i, j;
    if (m_missionMenu.pages) {
        for (i = 0; i < m_missionMenu.numPages; i++) {
            for (j = 0; j < 3; j++) {
                ui_element_free(m_missionMenu.pages[i][j]);
            }
            free(m_missionMenu.pages[i]);
        }
        free(m_missionMenu.pages);
        m_missionMenu.pages = NULL;
    }
}

