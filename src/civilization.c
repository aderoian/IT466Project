#include "simple_logger.h"

#include "gfc_input.h"
#include "gf2d_font.h"

#include "ui.h"
#include "def.h"
#include "civilization.h"

typedef struct CivilizationList_s {
    Civilization *civilizations;
    int count;
} CivilizationList;

typedef struct TradeMenu_s {
    int numPages;
    int currPage;
    UIElement *baseUI;
    UIElement ***pages;
    const Civilization* civ;
} TradeMenu;

CivilizationList g_civilizationList = {0};
static TradeMenu c_tradeMenu = {0};

void civilization_trade_update(float deltaTime);
void civilization_trade_draw();
 
void civilization_init() {
    DefinitionData *def, *cListDef, *cDef, *cResDef, *cMisDef, *cTradeDef;
    Civilization* civ;
    CivilTrade *trans;
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
            cTradeDef = def_data_array_get_nth(cResDef, j);
            if (!cTradeDef) continue;

            trans = (CivilTrade*)gfc_allocate_array(sizeof(CivilTrade), 1);
            resource_amount_from_config(def_data_get_obj(cTradeDef, "give"), &trans->give);
            resource_amount_from_config(def_data_get_obj(cTradeDef, "take"), &trans->take);
            gfc_list_append(civ->trades, trans);
        }

        cMisDef = def_data_get_array(cDef, "missions");
        def_data_array_get_count(cMisDef, &count);
        civ->missions = count > 0 ? gfc_list_new_size(count) : NULL;
        for (j = 0; j < count; j++) {
            
        }

        civ->model = strdup(def_data_get_string(cDef, "model"));
        civ->texture = strdup(def_data_get_string(cDef, "texture"));
    }

    c_tradeMenu.baseUI = ui_element_create_simple("images/ui/trade/trade_background.png", gfc_vector2d(0, 0));
    c_tradeMenu.baseUI->update = civilization_trade_update;
    c_tradeMenu.baseUI->draw = civilization_trade_draw;
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

void civilization_trade_create_pages(const Civilization* civ) {
    int numTrades, numPages, i, j, trade;
    GFC_Vector2D pos;
    if (!civ || !civ->trades) {
        slog ("civ has no trade list");
        c_tradeMenu.pages = NULL;
        c_tradeMenu.numPages = 0;
        return;
    }

    numTrades = gfc_list_count(civ->trades);
    numPages = numTrades % 3 == 0 ? numTrades / 3 : (numTrades / 3) + 1;
    slog("creating %d pages for %d trades", numPages, numTrades);
    c_tradeMenu.pages = (UIElement***) gfc_allocate_array(sizeof(UIElement**), numPages);
    for (i = 0; i < numPages; i++) {
        pos = gfc_vector2d(284, 275);
        c_tradeMenu.pages[i] = (UIElement**) gfc_allocate_array(sizeof(UIElement*), 3);
        for (j = 0; j < 3; j++) {
            trade = i * 3 + j;
            slog("trade %d", trade);
            if (trade >= numTrades) break;
            pos.x += i * 256;
            c_tradeMenu.pages[i][j] = ui_element_create_simple("images/ui/trade/trade_card.png", pos);
            // TODO: Store trade data so we can track which trade this card represents
            // TODO: This page element has children (resource image, and give/take amounts)
        }
    }

    for (i = 0; i < numPages; i++) {
        for (j = 0; j < 3; j++) {
            slog("card: %p", c_tradeMenu.pages[i][j]);
        }
    }

    c_tradeMenu.numPages = numPages;
}

void civilization_trade_open(const Civilization* civ) {
    if (!civ) return;

    c_tradeMenu.civ = civ;
    civilization_trade_create_pages(civ);
    c_tradeMenu.currPage = 0;

    if (!c_tradeMenu.pages) {
        return; // TODO: Send message that there are no trades.
    }
    ui_open_menu(c_tradeMenu.baseUI);
}

void civilization_trade_update(float deltaTime) {
    static int print = 0;
    if (!c_tradeMenu.pages) return;

    if (gfc_input_command_pressed("pagenext") && c_tradeMenu.currPage < c_tradeMenu.numPages - 1) {
        ++c_tradeMenu.currPage;
    } 
    if (gfc_input_command_pressed("pagelast") && c_tradeMenu.currPage > 0) {
        --c_tradeMenu.currPage;
    }

    // c_tradeMenu.baseUI->children = c_tradeMenu.pages[c_tradeMenu.currPage];
    // for (int j = 0; j < 3 && !print; j++) {
    //     slog("card: %p", c_tradeMenu.baseUI->children[j]);
    // }
    // print++;
    // c_tradeMenu.baseUI->childCount = 3;
}

void civilization_trade_draw() {
    char buffer[50];
    int i;
    gf2d_sprite_draw_image(c_tradeMenu.baseUI->sprite, c_tradeMenu.baseUI->position);
    // for (i = 0; i < c_tradeMenu.baseUI->childCount; i++) {
    //    if (c_tradeMenu.baseUI->children[i]) {
    //        ui_draw_element(c_tradeMenu.baseUI->children[i]);
    //    }
    // }
    for (i = 0; i < 3; i++) {
        if (c_tradeMenu.pages[c_tradeMenu.currPage][i]) {
            ui_draw_element(c_tradeMenu.pages[c_tradeMenu.currPage][i]);
        }
    }

    gf2d_font_draw_line_tag(c_tradeMenu.civ->name, FT_Normal, GFC_COLOR_WHITE, gfc_vector2d(539, 145));
    sprintf(buffer, "Page %d/%d", c_tradeMenu.currPage + 1, c_tradeMenu.numPages);
    gf2d_font_draw_line_tag(buffer, FT_Normal, GFC_COLOR_WHITE, gfc_vector2d(538, 558));
}