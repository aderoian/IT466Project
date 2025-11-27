#include "simple_logger.h"

#include "gfc_input.h"
#include "gf2d_mouse.h"
#include "gf2d_font.h"

#include "ui.h"
#include "def.h"
#include "civilization.h"
#include "player.h"

typedef struct CivilizationList_s {
    Civilization *civilizations;
    int count;
} CivilizationList;

typedef struct CivilMenu_s {
    int numPages;
    int currPage;
    UIElement *baseUI;
    UIElement ***pages;
    const Civilization* civ;
} CivilMenu;

typedef struct CardData_s {
    CivilTransaction* trans;
    GFC_Vector2D pos;
} CardData;

CivilizationList g_civilizationList = {0};
static CivilMenu c_tradeMenu = {0};
static CivilMenu c_missionMenu = {0};

void civilization_trade_update(float deltaTime);
void civilization_trade_draw();
void civilization_trade_click();
void civilization_trade_close();
void civilization_mission_update(float deltaTime);
void civilization_mission_draw();
void civilization_mission_click();
void civilization_mission_close();
 
void civilization_init() {
    DefinitionData *def, *cListDef, *cDef, *cResDef, *cMisDef, *cTransDef;
    Civilization* civ;
    CivilTransaction *trans;
    int i, j, count, type;
    def = def_load("defs/civilizations.def");
    if (!def) {
        slog("Failed to load civilizations.def\n");
        return;
    }

    cListDef = def_data_get_array(def, "civilizations");
    if (!cListDef) {
        slog("No civilizations found in civilizations.def\n");
        return;
    }

    def_data_array_get_count(cListDef, &g_civilizationList.count);
    g_civilizationList.civilizations = malloc(sizeof(Civilization) * g_civilizationList.count);
    for (i = 0; i < g_civilizationList.count; i++) {
        civ = &g_civilizationList.civilizations[i];
        cDef = def_data_array_get_nth(cListDef, i);

        civ->name = strdup(def_data_get_string(cDef, "name"));
        def_data_get_int(cDef, "type", &type);
        switch(type) {
            case 1:
                civ->type = CIVIL_TYPE_MINING;
                break;
            default:
                slog("Got unknown civilization type: %d", type);
                civ->type = CIVIL_TYPE_UNKNOWN;
        }

        cResDef = def_data_get_array(cDef, "trades");
        def_data_array_get_count(cResDef, &count);
        civ->trades = count > 0 ? gfc_list_new_size(count) : NULL;
        for (j = 0; j < count; j++) {
            cTransDef = def_data_array_get_nth(cResDef, j);
            if (!cTransDef) continue;

            trans = (CivilTransaction*)gfc_allocate_array(sizeof(CivilTransaction), 1);
            resource_amount_from_config(def_data_get_obj(cTransDef, "give"), &trans->give);
            resource_amount_from_config(def_data_get_obj(cTransDef, "take"), &trans->take);
            gfc_list_append(civ->trades, trans);
        }

        cMisDef = def_data_get_array(cDef, "missions");
        def_data_array_get_count(cMisDef, &count);
        civ->missions = count > 0 ? gfc_list_new_size(count) : NULL;
        for (j = 0; j < count; j++) {
            cTransDef = def_data_array_get_nth(cMisDef, j);
            if (!cTransDef) continue;

            trans = (CivilTransaction*)gfc_allocate_array(sizeof(CivilTransaction), 1);
            resource_amount_from_config(def_data_get_obj(cTransDef, "give"), &trans->give);
            resource_amount_from_config(def_data_get_obj(cTransDef, "take"), &trans->take);
            gfc_list_append(civ->missions, trans);
        }

        civ->model = strdup(def_data_get_string(cDef, "model"));
        civ->texture = strdup(def_data_get_string(cDef, "texture"));
    }

    c_tradeMenu.baseUI = ui_element_create_simple("images/ui/trade/trade_background.png", gfc_vector2d(0, 0));
    c_tradeMenu.baseUI->update = civilization_trade_update;
    c_tradeMenu.baseUI->draw = civilization_trade_draw;
    c_tradeMenu.baseUI->onClick = civilization_trade_click;
    c_tradeMenu.baseUI->onClose = civilization_trade_close;

    c_missionMenu.baseUI = ui_element_create_simple("images/ui/mission/mission_background.png", gfc_vector2d(0, 0));
    c_missionMenu.baseUI->update = civilization_mission_update;
    c_missionMenu.baseUI->draw = civilization_mission_draw;
    c_missionMenu.baseUI->onClick = civilization_mission_click;
    c_missionMenu.baseUI->onClose = civilization_mission_close;
}

const Civilization* civilization_get_by_name(const char *name) {
    int i;
    for (i = 0; i < g_civilizationList.count; i++) {
        if (strcmp(g_civilizationList.civilizations[i].name, name) == 0)
            return &g_civilizationList.civilizations[i];
    }
    return NULL;
}

Entity* civilization_spawn(GFC_Vector3D pos, const Civilization* civilization) {
    CivilizationEntityData *data = NULL;
    Entity* self;
    if (!civilization) return NULL;
    
    self = entity_new();
    if (!self) return NULL;

    gfc_line_cpy(self->name, "civilization");
    self->mesh = gf3d_mesh_load(civilization->model);
    self->texture = gf3d_texture_load(civilization->texture);
    self->color = GFC_COLOR_WHITE;
    self->position = pos;

    data = (CivilizationEntityData*) gfc_allocate_array(sizeof(CivilizationEntityData), 1);
    data->civilization = civilization;
    self->data = data;

    self->physicsBody = physics_body_create();
    self->physicsBody->position = pos;
    self->physicsBody->mass = 10;
    self->physicsBody->invMass = 0;
    self->physicsBody->owner = self;
    self->physicsBody->flags |= FLAG_NO_COLLISION_RESOLUTION;
    self->physicsBody->shape.type = FLAG_AABB;
    gfc_vector2d_copy(self->physicsBody->shape.Shape.aabb[0], gfc_vector3d(-1000, -1000, -1000));
    gfc_vector2d_copy(self->physicsBody->shape.Shape.aabb[1], gfc_vector3d(1000, 1000, 1000));
    return self;
}

void civilization_menu_create_pages(CivilMenu* menu, GFC_List* items) {
    int numTrans, numPages, i, j, transIdx;
    CivilTransaction* trans;
    CardData* data;
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
            trans = gfc_list_get_nth(items, transIdx);

            pos.x = 284 + j * 256;
            menu->pages[i][j] = ui_element_create_simple("images/ui/trade/trade_card.png", pos);
            data = gfc_allocate_array(sizeof(CardData), 1);
            data->pos = pos;
            data->trans = trans;
            menu->pages[i][j]->data = data;
            
            gfc_vector2d_add(iconPos, pos, gfc_vector2d(25, 25));
            menu->pages[i][j]->children = gfc_allocate_array(sizeof(UIElement*), 1);
            menu->pages[i][j]->children[0] = ui_element_create_simple(trans->take.resource->icon, iconPos);
            menu->pages[i][j]->childCount = 1;
        }
    }

    menu->numPages = numPages;
}

void civilization_trade_open(const Civilization* civ) {
    if (!civ) return;

    c_tradeMenu.civ = civ;
    civilization_menu_create_pages(&c_tradeMenu, civ->trades);
    c_tradeMenu.currPage = 0;

    if (!c_tradeMenu.pages) {
        return; // TODO: Send message that there are no trades.
    }
    ui_open_menu(c_tradeMenu.baseUI);
}

void civilization_mission_open(const Civilization* civ) {
    if (!civ) return;

    c_missionMenu.civ = civ;
    civilization_menu_create_pages(&c_missionMenu, civ->missions);
    c_missionMenu.currPage = 0;

    if (!c_missionMenu.pages) {
        return; // TODO: Send message that there are no trades.
    }
    ui_open_menu(c_missionMenu.baseUI);
}

void civilization_trade_with(const Civilization* civ, const CivilTransaction* trade) {
    if (!civ || !trade || !player) return;

    if (player_try_take_resource(player, &trade->take)) {
        player_give_resource(player, &trade->give);
    }
}

void civilization_trade_update(float deltaTime) {
    int delta = 0;
    if (!c_tradeMenu.pages) return;

    if (gfc_input_command_pressed("pagenext")) delta += 1;
    if (gfc_input_command_pressed("pagelast")) delta -= 1;
    c_tradeMenu.currPage = (c_tradeMenu.currPage + delta < 0 ? 0 : c_tradeMenu.currPage + delta) % c_tradeMenu.numPages;
}

void civilization_mission_update(float deltaTime) {
    int delta = 0;
    if (!c_missionMenu.pages) return;

    if (gfc_input_command_pressed("pagenext")) delta += 1;
    if (gfc_input_command_pressed("pagelast")) delta -= 1;
    c_missionMenu.currPage = (c_missionMenu.currPage + delta < 0 ? 0 : c_missionMenu.currPage + delta) % c_missionMenu.numPages;
}

void civilization_trade_draw() {
    char buffer[50];
    int i;
    UIElement *tradeCard;
    CardData *data;
    GFC_Vector2D pos;
    gf2d_sprite_draw_image(c_tradeMenu.baseUI->sprite, c_tradeMenu.baseUI->position);
    for (i = 0; i < 3; i++) {
        tradeCard = c_tradeMenu.pages[c_tradeMenu.currPage][i];
        if (tradeCard) {
            ui_draw_element(tradeCard);
            data = (CardData*) tradeCard->data;
            gfc_vector2d_add(pos, data->pos, gfc_vector2d(25, 185));
            sprintf(buffer, "Give:");
            gf2d_font_draw_line_tag(buffer, FT_Small, GFC_COLOR_WHITE, pos);
            pos.y += 20;
            sprintf(buffer, "%s x%d", data->trans->give.resource->name, data->trans->give.amount);
            gf2d_font_draw_line_tag(buffer, FT_Small, GFC_COLOR_WHITE, pos);

            pos.y += 30;
            sprintf(buffer, "Take:");
            gf2d_font_draw_line_tag(buffer, FT_Small, GFC_COLOR_WHITE, pos);
            pos.y += 20;
            sprintf(buffer, "%s x%d", data->trans->take.resource->name, data->trans->take.amount);
            gf2d_font_draw_line_tag(buffer, FT_Small, GFC_COLOR_WHITE, pos);
        }
    }

    gf2d_font_draw_line_tag(c_tradeMenu.civ->name, FT_Normal, GFC_COLOR_WHITE, gfc_vector2d(539, 145));
    sprintf(buffer, "Page %d/%d", c_tradeMenu.currPage + 1, c_tradeMenu.numPages);
    gf2d_font_draw_line_tag(buffer, FT_Normal, GFC_COLOR_WHITE, gfc_vector2d(538, 558));
}

void civilization_mission_draw() {
    char buffer[50];
    int i;
    UIElement *tradeCard;
    CardData *data;
    GFC_Vector2D pos;
    gf2d_sprite_draw_image(c_missionMenu.baseUI->sprite, c_missionMenu.baseUI->position);
    for (i = 0; i < 3; i++) {
        tradeCard = c_missionMenu.pages[c_missionMenu.currPage][i];
        if (tradeCard) {
            ui_draw_element(tradeCard);
            data = (CardData*) tradeCard->data;
            gfc_vector2d_add(pos, data->pos, gfc_vector2d(25, 185));
            sprintf(buffer, "Collect:");
            gf2d_font_draw_line_tag(buffer, FT_Small, GFC_COLOR_WHITE, pos);
            pos.y += 20;
            sprintf(buffer, "%s x%d", data->trans->take.resource->name, data->trans->take.amount);
            gf2d_font_draw_line_tag(buffer, FT_Small, GFC_COLOR_WHITE, pos);

            pos.y += 30;
            sprintf(buffer, "Reward:");
            gf2d_font_draw_line_tag(buffer, FT_Small, GFC_COLOR_WHITE, pos);
            pos.y += 20;
            sprintf(buffer, "%s x%d", data->trans->give.resource->name, data->trans->give.amount);
            gf2d_font_draw_line_tag(buffer, FT_Small, GFC_COLOR_WHITE, pos);
        }
    }

    gf2d_font_draw_line_tag(c_missionMenu.civ->name, FT_Normal, GFC_COLOR_WHITE, gfc_vector2d(539, 145));
    sprintf(buffer, "Page %d/%d", c_missionMenu.currPage + 1, c_missionMenu.numPages);
    gf2d_font_draw_line_tag(buffer, FT_Normal, GFC_COLOR_WHITE, gfc_vector2d(538, 558));
}

void civilization_trade_click() {
    int i;
    UIElement* card;
    if (c_tradeMenu.numPages == 0 || !c_tradeMenu.pages || !c_tradeMenu.pages[c_tradeMenu.currPage]) return;
    for (i = 0; i < 3; i++) {
        card = c_tradeMenu.pages[c_tradeMenu.currPage][i];
        if (!card) break;

        if (gf2d_mouse_in_rect(card->localBounding)) {
            civilization_trade_with(c_tradeMenu.civ, ((CardData*)card->data)->trans);
            ui_close_menu();
            return;
        }
    }
}

void civilization_mission_click() {
    int i;
    UIElement* card;
    CivilMission *mission;
    if (c_missionMenu.numPages == 0 || !c_missionMenu.pages || !c_missionMenu.pages[c_missionMenu.currPage]) return;
    for (i = 0; i < 3; i++) {
        card = c_missionMenu.pages[c_missionMenu.currPage][i];
        if (!card) break;

        if (player && gf2d_mouse_in_rect(card->localBounding)) {
            mission = gfc_allocate_array(sizeof(CivilMission), 1);
            mission->civilization = c_missionMenu.civ;
            mission->trans = ((CardData*)card->data)->trans;
            player_start_mission(player, mission);
            ui_close_menu();
            return;
        }
    }
}

void civilization_trade_close() {
    int i, j;
    if (c_tradeMenu.pages) {
        for (i = 0; i < c_tradeMenu.numPages; i++) {
            for (j = 0; j < 3; j++) {
                ui_element_free(c_tradeMenu.pages[i][j]);
            }
            free(c_tradeMenu.pages[i]);
        }
        free(c_tradeMenu.pages);
        c_tradeMenu.pages = NULL;
    }
}

void civilization_mission_close() {
    int i, j;
    if (c_missionMenu.pages) {
        for (i = 0; i < c_missionMenu.numPages; i++) {
            for (j = 0; j < 3; j++) {
                ui_element_free(c_missionMenu.pages[i][j]);
            }
            free(c_missionMenu.pages[i]);
        }
        free(c_missionMenu.pages);
        c_missionMenu.pages = NULL;
    }
}