#include "simple_logger.h"
#include "simple_json.h"

#include "gf2d_font.h"

#include "def.h"
#include "ui.h"
#include "entity.h"

#include "building.h"
#include "player.h"

typedef struct BuildingList_s {
    const Building *buildings;
    int count;
} BuildingList;

typedef struct BuildingEntityData_s {
    const Building* building;
    int tick;
} BuildingEntityData;

typedef struct BuildingMenu_s {
    int numPages;
    int currPage;
    UIElement *baseUI;
    UIElement ***pages;

    // Passthroughs
    Entity *planet;
    GFC_Vector3D pos;
} BuildingMenu;

typedef struct BuildingCardData_s {
    const Building* building;
    GFC_Vector2D pos;
} BuildingCardData;

BuildingList g_buildingList = {0};
static BuildingMenu m_buildingMenu = {0};

void building_load_production_from_def(SJson* json, BuildingProduction *prod);

void building_menu_init();

void building_init() {
    DefinitionData *def, *bListDef, *bDef, *bProdList, *bProd, *bCostList;
    Building* building;
    int i, j;
    def = def_load("defs/buildings.def");
    if (!def) {
        slog("Failed to load buildings.def\n");
        return;
    }

    bListDef = def_data_get_array(def, "buildings");
    if (!bListDef) {
        slog("No buildings found in buildings.def\n");
        return;
    }

    def_data_array_get_count(bListDef, &g_buildingList.count);
    g_buildingList.buildings = malloc(sizeof(Building) * g_buildingList.count);
    for (i = 0; i < g_buildingList.count; i++) {
        building = &g_buildingList.buildings[i];
        bDef = def_data_array_get_nth(bListDef, i);
        building->name = strdup(def_data_get_string(bDef, "name"));
        building->mesh = strdup(def_data_get_string(bDef, "model"));
        building->texture = strdup(def_data_get_string(bDef, "texture"));
        
        bProdList = def_data_get_array(bDef, "products");
        def_data_array_get_count(bProdList, &building->productCount);
        building->production = gfc_allocate_array(sizeof(BuildingProduction), building->productCount);
        for (j = 0; j < building->productCount; j++) {
            bProd = def_data_array_get_nth(bProdList, j);
            building_load_production_from_def(bProd, &building->production[j]);
        }

        bCostList = def_data_get_array(bDef, "buildCost");
        def_data_array_get_count(bCostList, &building->costAmount);
        building->cost = gfc_allocate_array(sizeof(ResourceAmount), building->costAmount);
        for (j = 0; j < building->costAmount; j++) {
            resource_amount_from_config(def_data_array_get_nth(bCostList, j), &building->cost[j]);
        }
    }

    building_menu_init();
}

const Building* building_get_by_name(const char *name) {
    int i;
    for (i = 0; i < g_buildingList.count; i++) {
        if (strcmp(g_buildingList.buildings[i].name, name) == 0)
            return &g_buildingList.buildings[i];
    }
    return NULL;
}

void building_load_production_from_def(SJson* json, BuildingProduction *prod) {
    if (!json || !prod) return;
    resource_amount_from_config(def_data_get_obj(json, "production"), &prod->production);
    resource_amount_from_config(def_data_get_obj(json, "productionCost"), &prod->productionCost);
    def_data_get_int(json, "duration", &prod->productionDuration);
}

void building_entity_think(Entity* ent) {
    BuildingEntityData *data;
    BuildingProduction *prod;
    int i;
    if (!ent || !player) return;

    data = (BuildingEntityData*) ent->data;
    data->tick++;
    for (i = 0; i < data->building->productCount; i++) {
        prod = &data->building->production[i];
        if (data->tick % prod->productionDuration == 0) {
            if (prod->productionCost.amount <= 0 || player_try_take_resource(player, &prod->productionCost))
                player_try_give_resource(player, &prod->production);
        }
    }
}

Entity* building_spawn_entity(Entity* planet, const Building* building, GFC_Vector3D surfacePosition) {
    GFC_Vector3D dir;
    Entity* self;
    BuildingEntityData *data;
    if (!planet || !building) return NULL;

    self = entity_new();
    strcpy(self->name, "building");
    self->mesh = gf3d_mesh_load(building->mesh);
    if (!self->mesh) return NULL;
    self->position = surfacePosition;
    self->texture = gf3d_texture_load(building->texture);
    self->rotation = quaternion_create(0, 0, 0, 1);
    self->color = GFC_COLOR_WHITE;
    self->scale = gfc_vector3d(3, 3, 3);

    data = gfc_allocate_array(sizeof(BuildingEntityData), 1);
    data->building = building;
    self->data = data;

    self->think = building_entity_think;

    return self;
}

void building_menu_update(float deltaTime);
void building_menu_draw();
void building_menu_click();
void building_menu_rightClick();
void building_menu_close();

void building_menu_init() {
    m_buildingMenu.baseUI = ui_element_create_simple("images/ui/building/building_background.png", gfc_vector2d(0, 0));
    m_buildingMenu.baseUI->update = building_menu_update;
    m_buildingMenu.baseUI->draw = building_menu_draw;
    m_buildingMenu.baseUI->onClick = building_menu_click;

    building_menu_create_pages(&m_buildingMenu, g_buildingList.buildings, g_buildingList.count);
}

void building_menu_free_pages(BuildingMenu* menu) {
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

void building_menu_create_pages(BuildingMenu* menu, const Building* buildings, Uint32 count) {
    int numPages, i, j, transIdx;
    BuildingCardData* data;
    GFC_Vector2D pos;
    if (!menu) return;

    if (menu->pages) building_menu_free_pages(menu); 
    if (!buildings) {
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
            menu->pages[i][j] = ui_element_create_simple("images/ui/building/building_card.png", pos);
            data = gfc_allocate_array(sizeof(BuildingCardData), 1);
            data->pos = pos;
            data->building = &buildings[transIdx];
            menu->pages[i][j]->data = data;
        }
    }

    menu->numPages = numPages;
}

void building_menu_open(Entity *planet, GFC_Vector3D pos) {
    if (!player) return;
    m_buildingMenu.currPage = 0;
    m_buildingMenu.planet = planet;
    m_buildingMenu.pos = pos;

    if (!m_buildingMenu.pages) {
        return; // TODO: Send message that there are no trades.
    }
    ui_open_menu(m_buildingMenu.baseUI);
}

void building_menu_update(float deltaTime) {
    int delta = 0;
    if (!m_buildingMenu.pages) return;

    if (gfc_input_command_pressed("pagenext")) delta += 1;
    if (gfc_input_command_pressed("pagelast")) delta -= 1;
    m_buildingMenu.currPage = (m_buildingMenu.currPage + delta < 0 ? 0 : m_buildingMenu.currPage + delta) % m_buildingMenu.numPages;
}

void building_menu_draw() {
    char buffer[50];
    int i, j;
    UIElement *card;
    BuildingCardData *data;
    BuildingProduction *prod;
    GFC_Vector2D pos;
    gf2d_sprite_draw_image(m_buildingMenu.baseUI->sprite, m_buildingMenu.baseUI->position);
    for (i = 0; i < 3; i++) {
        card = m_buildingMenu.pages[m_buildingMenu.currPage][i];
        if (card) {
            ui_draw_element(card);
            data = (BuildingCardData*) card->data;
            gfc_vector2d_add(pos, data->pos, gfc_vector2d(18, 12));

            sprintf(buffer, "%s", data->building->name);
            gf2d_font_draw_line_tag(buffer, FT_Normal, GFC_COLOR_WHITE, pos);
            pos.y += 40;

            for (j = 0; j < data->building->productCount; j++) {
                prod = &data->building->production[j];
                sprintf(buffer, "Product: x%d %s", prod->production.amount, prod->production.resource->name);
                gf2d_font_draw_line_tag(buffer, FT_Small, GFC_COLOR_WHITE, pos);
                pos.y += 20;
                if (prod->productionCost.amount > 0) {
                    sprintf(buffer, "Product Cost: x%d %s", prod->productionCost.amount, prod->productionCost.resource->name);
                    gf2d_font_draw_line_tag(buffer, FT_Small, GFC_COLOR_WHITE, pos);
                    pos.y += 20;
                }

                pos.y += 20;
            }
        }
    }

    sprintf(buffer, "Page %d/%d", m_buildingMenu.currPage + 1, m_buildingMenu.numPages);
    gf2d_font_draw_line_tag(buffer, FT_Normal, GFC_COLOR_WHITE, gfc_vector2d(538, 558));
}

void building_menu_click() {
    int i;
    UIElement* card;
    BuildingCardData *data;
    if (m_buildingMenu.numPages == 0 || !m_buildingMenu.pages || !m_buildingMenu.pages[m_buildingMenu.currPage]) return;
    for (i = 0; i < 3; i++) {
        card = m_buildingMenu.pages[m_buildingMenu.currPage][i];
        if (!card) break;

        if (player && gf2d_mouse_in_rect(card->localBounding)) {
            data = (BuildingCardData*)card->data;
            ui_close_menu();
            player_try_build(player, data->building, m_buildingMenu.planet, m_buildingMenu.pos);
            return;
        }
    }
}

void building_menu_free() {
    building_menu_free_pages(&m_buildingMenu);
}