#include "simple_logger.h"

#include "gf2d_mouse.h"

#include "ui.h"
#include "world.h"

#include "world_map.h"

typedef struct ViewIconData_s {
    Uint32 index;
} ViewIconData;

typedef enum {
    UNIVERSE_VIEW,
    GALAXY_VIEW,
    SOLAR_SYSTEM_VIEW
} MapView;

typedef struct WorldMapManager_s {
    MapView view;
    UIElement* baseUI;
    UIElement* locationSelector;
    UIElement** universeView;
    int galaxies;
    UIElement** galaxyView;
    int galaxy;
    int solarSystems;
    UIElement** solarSystemView;
    int celestialBodies;
} WorldMapManager;

static WorldMapManager map_manager = {0};

void world_map_unload_view_elements(UIElement** el, int count);

void world_map_draw() {
    gf2d_sprite_draw_image(map_manager.baseUI->sprite, map_manager.baseUI->position);
    for (int i = 0; i < map_manager.baseUI->childCount; i++) {
        ui_draw_element(map_manager.baseUI->children[i]);
    }
    ui_draw_element(map_manager.locationSelector);
}

void world_map_unload_view_elements(UIElement** el, int count) {
    int i;
    if (!el) return;
    for (i = 0; i < count; i++) {
        if (!el[i]) continue;
        if (el[i]->data) free(el[i]->data);
        gf2d_sprite_free(el[i]->sprite);
        free(el[i]);
    }
    free(el);
}

void world_map_load_solarsystems(Galaxy* galaxy) {
    int i;
    UIElement** galaxyView;
    ViewIconData* data;
    GFC_Vector2D position;

    slog("Loading solar systems: %d", galaxy->numSolarSystems);

    if (!galaxy || galaxy->numSolarSystems <= 0 ||!galaxy->solarSystems) return;
    if (map_manager.galaxyView) {
        world_map_unload_view_elements(map_manager.galaxyView, map_manager.solarSystems);
        map_manager.baseUI->children = NULL;
        map_manager.baseUI->childCount = 0;
        map_manager.galaxyView = NULL;
    }

    galaxyView = (UIElement**) gfc_allocate_array(sizeof(UIElement*), galaxy->numSolarSystems);
    for (i = 1; i < galaxy->numSolarSystems; i++) {
        gfc_vector2d_add(position, galaxy->solarSystems[i]->pos, gfc_vector2d(100, 61));
        galaxyView[i]  = ui_element_create_simple("images/ui/map/solar_system_icon.png", position);
        data = gfc_allocate_array(sizeof(ViewIconData), 1);
        data->index = i;
        galaxyView[i]->data = data;
    }
    map_manager.galaxyView = galaxyView;
    map_manager.baseUI->children = galaxyView;
    map_manager.baseUI->childCount = galaxy->numSolarSystems;
    map_manager.solarSystems = galaxy->numSolarSystems;
}

void world_map_load_solarsystem(SolarSystem* solarSystem) {
    int i;
    UIElement** solarSystemView;
    ViewIconData* data;
    GFC_Vector2D position;
    if (!solarSystem || solarSystem->numBodies <= 0 ||!solarSystem->celestialBodies) return;
    if (map_manager.solarSystemView) {
        world_map_unload_view_elements(map_manager.solarSystemView, map_manager.celestialBodies);
        map_manager.baseUI->children = NULL;
        map_manager.baseUI->childCount = 0;
        map_manager.solarSystemView = NULL;
    }

    solarSystemView = (UIElement**) gfc_allocate_array(sizeof(UIElement*), solarSystem->numBodies);
    for (i = 1; i < solarSystem->numBodies; i++) {
        gfc_vector2d_add(position, solarSystem->celestialBodies[i]->pos, gfc_vector2d(640, 360));
        solarSystemView[i]  = ui_element_create_simple("images/ui/map/planet_icon.png", position);
        data = gfc_allocate_array(sizeof(ViewIconData), 1);
        data->index = i;
        solarSystemView[i]->data = data;
    }
    map_manager.solarSystemView = solarSystemView;
    map_manager.baseUI->children = solarSystemView;
    map_manager.baseUI->childCount = solarSystem->numBodies;
    map_manager.celestialBodies = solarSystem->numBodies;
}

void world_map_on_click() {
    UIElement* el;
    Uint32 index;
    int i;
    for (i = 0; i < map_manager.baseUI->childCount; i++) {
        el = map_manager.baseUI->children[i];
        if (el && gf2d_mouse_in_rect(el->localBounding)) {
            index = ((ViewIconData*) el->data)->index;
            slog("clicked index %d", index);
            if (map_manager.view == UNIVERSE_VIEW) {
                world_map_load_solarsystems(world_get_universe()->galaxies[index]);
                map_manager.galaxy = index;
                map_manager.view = GALAXY_VIEW;
            } else if (map_manager.view == GALAXY_VIEW){
                world_map_load_solarsystem(world_get_universe()->galaxies[map_manager.galaxy]->solarSystems[index]);
                map_manager.view = SOLAR_SYSTEM_VIEW;
            }
            return;
        }
    }
}

void world_map_on_hover() {
    UIElement* el;
    int i;
    for (i = 0; i < map_manager.baseUI->childCount; i++) {
        el = map_manager.baseUI->children[i];
        if (el && gf2d_mouse_in_rect(el->localBounding)) {
            map_manager.locationSelector->position.x = el->position.x - 9;
            map_manager.locationSelector->position.y = el->position.y - 6;
            break;
        }
    }
}

void world_map_on_close() {
    if (map_manager.view != UNIVERSE_VIEW) {
        map_manager.view = UNIVERSE_VIEW;
        world_map_unload_view_elements(map_manager.galaxyView, map_manager.solarSystems);
        map_manager.solarSystems = 0;
        map_manager.galaxyView = NULL;
        world_map_unload_view_elements(map_manager.solarSystemView, map_manager.celestialBodies);
        map_manager.celestialBodies = 0;
        map_manager.solarSystemView = NULL;

        map_manager.baseUI->children = map_manager.universeView;
        map_manager.baseUI->childCount = map_manager.galaxies;
    }
}

void world_map_load() {
    int i;
    Universe* univ;
    UIElement **univMap;
    ViewIconData* data;
    GFC_Vector2D position;

    map_manager.view = UNIVERSE_VIEW;
    map_manager.baseUI = ui_element_create_simple("images/ui/map/map_background.png", gfc_vector2d(0, 0));

    map_manager.baseUI->draw = world_map_draw;
    map_manager.baseUI->onClick = world_map_on_click;
    map_manager.baseUI->onHover = world_map_on_hover;
    map_manager.baseUI->onClose = world_map_on_close;
    map_manager.locationSelector = ui_element_create_simple("images/ui/map/location_selector.png", gfc_vector2d(-9 + 100, -8 + 61));

    univ = world_get_universe();

    slog("num galaxies: %d", univ->numGalaxies);
    univMap = (UIElement**) gfc_allocate_array(sizeof(UIElement*), univ->numGalaxies);
    for (i = 0; i < univ->numGalaxies; i++) {
        gfc_vector2d_add(position, univ->galaxies[i]->pos, gfc_vector2d(100, 61));
        univMap[i]  = ui_element_create_simple("images/ui/map/galaxy_icon.png", position);
        data = gfc_allocate_array(sizeof(ViewIconData), 1);
        data->index = i;
        univMap[i]->data = data;
        slog("galaxy %d pos: %f %f", i, position.x, position.y);
    }
    map_manager.universeView = univMap;
    map_manager.baseUI->children = univMap;
    map_manager.baseUI->childCount = univ->numGalaxies;
    map_manager.galaxies = univ->numGalaxies;
    GFC_TextLine name = "openmap";
    register_command_callback(name, world_map_get);
}

struct UIElement_s* world_map_get() {
    return map_manager.baseUI;
}
