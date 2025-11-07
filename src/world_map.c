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
    UIElement* universeView;
    UIElement* galaxyView;
    int galaxy;
    UIElement* solarSystemView;
} WorldMapManager;

static WorldMapManager map_manager = {0};

void world_map_unload_view_element(UIElement* el);

void world_map_draw() {
    UIElement* el;
    gf2d_sprite_draw_image(map_manager.baseUI->sprite, map_manager.baseUI->position);
    gf2d_sprite_draw_image(map_manager.locationSelector->sprite, map_manager.locationSelector->position); //TODO: move based off player location


    switch (map_manager.view) {
        case UNIVERSE_VIEW:
            el = map_manager.universeView;
            break;
        case GALAXY_VIEW:
            el = map_manager.galaxyView;
            break;
        case SOLAR_SYSTEM_VIEW:
            el = map_manager.solarSystemView;
    }
    while (el) {
        gf2d_sprite_draw_image(el->sprite, el->position);
        el = el->child;
    }
}

void world_map_unload_view_element(UIElement* el) {
    if (!el) return;
    if (el->child) {
        world_map_unload_view_element(el->child);
    }

    free(el->data);
    gf2d_sprite_free(el->sprite);
    free(el);
}

void world_map_load_solarsystems(Galaxy* galaxy) {
    int i;
    UIElement* icon;
    ViewIconData* data;
    GFC_Vector2D position;
    if (!galaxy || galaxy->numSolarSystems <= 0 ||!galaxy->solarSystems) return;
    if (map_manager.galaxyView) {
        world_map_unload_view_element(map_manager.galaxyView);
    }

    gfc_vector2d_add(position, galaxy->solarSystems[0]->pos, gfc_vector2d(100, 61));
    map_manager.galaxyView = ui_element_create_simple("images/ui/map/galaxy_icon.png", position);
    data = gfc_allocate_array(sizeof(ViewIconData), 1);
    data->index = 0;
    map_manager.galaxyView->data = data;

    icon = map_manager.galaxyView;
    for (i = 1; i < galaxy->numSolarSystems; i++) {
        gfc_vector2d_add(position, galaxy->solarSystems[i]->pos, gfc_vector2d(100, 61));
        icon->child = ui_element_create_simple("images/ui/map/galaxy_icon.png", position);
        icon = icon->child;
        data = gfc_allocate_array(sizeof(ViewIconData), 1);
        data->index = i;
        icon->data = data;
    }
}

void world_map_load_solarsystem(SolarSystem* solarSystem) {
    int i;
    UIElement* icon;
    ViewIconData* data;
    GFC_Vector2D position;
    if (!solarSystem || solarSystem->numBodies <= 0 ||!solarSystem->celestialBodies) return;
    if (map_manager.solarSystemView) {
        world_map_unload_view_element(map_manager.solarSystemView);
    }

    gfc_vector2d_add(position, solarSystem->celestialBodies[0]->pos, gfc_vector2d(640, 360));
    map_manager.solarSystemView = ui_element_create_simple("images/ui/map/galaxy_icon.png", position);
    data = gfc_allocate_array(sizeof(ViewIconData), 1);
    data->index = 0;
    map_manager.solarSystemView->data = data;

    icon = map_manager.solarSystemView;
    for (i = 1; i < solarSystem->numBodies; i++) {
        gfc_vector2d_add(position, solarSystem->celestialBodies[i]->pos, gfc_vector2d(640, 360));
        icon->child = ui_element_create_simple("images/ui/map/galaxy_icon.png", position);
        icon = icon->child;
        data = gfc_allocate_array(sizeof(ViewIconData), 1);
        data->index = i;
        icon->data = data;
    }
}

void world_map_on_click() {
    UIElement* el;
    Uint32 index;
    switch (map_manager.view) {
        case UNIVERSE_VIEW:
            el = map_manager.universeView;
            break;
        case GALAXY_VIEW:
            el = map_manager.galaxyView;
            break;
        case SOLAR_SYSTEM_VIEW:
            el = map_manager.solarSystemView;
    }

    while (el) {
        if (gf2d_mouse_in_rect(el->localBounding)) {
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
            break;
        }
        el = el->child;
    }
}

void world_map_on_hover() {
    UIElement* el;
    switch (map_manager.view) {
        case UNIVERSE_VIEW:
            el = map_manager.universeView;
            break;
        case GALAXY_VIEW:
            el = map_manager.galaxyView;
            break;
        case SOLAR_SYSTEM_VIEW:
            el = map_manager.solarSystemView;
    }

    while (el) {
        if (gf2d_mouse_in_rect(el->localBounding)) {
            map_manager.locationSelector->position.x = el->position.x - 9;
            map_manager.locationSelector->position.y = el->position.y - 6;
            break;
        }
        el = el->child;
    }
}

void world_map_on_close() {
    if (map_manager.view != UNIVERSE_VIEW) {
        map_manager.view = UNIVERSE_VIEW;
        world_map_unload_view_element(map_manager.galaxyView);
        world_map_unload_view_element(map_manager.solarSystemView);
    }
}

void world_map_load() {
    int i;
    Universe* univ;
    UIElement* icon;
    ViewIconData* data;
    GFC_Vector2D position;

    map_manager.view = UNIVERSE_VIEW;
    map_manager.baseUI = ui_element_create_simple("images/ui/map/map_background.png", gfc_vector2d(0, 0));

    map_manager.baseUI->draw = world_map_draw;
    map_manager.baseUI->onClick = world_map_on_click;
    map_manager.baseUI->onHover = world_map_on_hover;
    map_manager.baseUI->onClose = world_map_on_close;
    map_manager.locationSelector = ui_element_create_simple("images/ui/map/location_selector.png", gfc_vector2d(-9 + 100, -8 + 61));

    slog("box %f %f", map_manager.locationSelector->localBounding.w, map_manager.locationSelector->localBounding.h);

    univ = world_get_universe();
    gfc_vector2d_add(position, univ->galaxies[0]->pos, gfc_vector2d(100, 61));
    map_manager.universeView = ui_element_create_simple("images/ui/map/galaxy_icon.png", position);
    data = gfc_allocate_array(sizeof(ViewIconData), 1);
    data->index = 0;
    map_manager.universeView->data = data;

    icon = map_manager.universeView;
    for (i = 1; i < univ->numGalaxies; i++) {
        gfc_vector2d_add(position, univ->galaxies[i]->pos, gfc_vector2d(100, 61));
        icon->child = ui_element_create_simple("images/ui/map/galaxy_icon.png", position);
        icon = icon->child;
        data = gfc_allocate_array(sizeof(ViewIconData), 1);
        data->index = i;
        icon->data = data;
    }


    GFC_TextLine name = "openmap";
    register_command_callback(name, world_map_get);
}

struct UIElement_s* world_map_get() {
    return map_manager.baseUI;
}
