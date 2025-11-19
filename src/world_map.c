#include "simple_logger.h"

#include "gf2d_mouse.h"

#include "ui.h"
#include "world.h"
#include "player.h"

#include "world_map.h"

#define DISTANCE_SCALE_FACTOR 120

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
    UIElement* playerMarker;
    UIElement* jumpTargetSelector;
    UIElement* jumpButton;
    UIElement** universeView;
    int galaxies;
    UIElement** galaxyView;
    int solarSystems;
    UIElement** solarSystemView;
    int celestialBodies;

    int galaxy;
    int solarSystem;
    int playerCurrentGalaxy;
    int playerCurrentSolarSystem;
    int jumpTarget;
} WorldMapManager;

static WorldMapManager map_manager = {0};

void world_map_unload_view_elements(UIElement** el, int count);

void world_map_draw() {
    Galaxy* galaxy;
    SolarSystem* solarSystem;
    GFC_Vector2D position;
    gf2d_sprite_draw_image(map_manager.baseUI->sprite, map_manager.baseUI->position);
    for (int i = 0; i < map_manager.baseUI->childCount; i++) {
        ui_draw_element(map_manager.baseUI->children[i]);
    }
    ui_draw_element(map_manager.locationSelector);

    if (map_manager.view == GALAXY_VIEW && map_manager.galaxy == map_manager.playerCurrentGalaxy) {
        galaxy = world_get_universe()->galaxies[map_manager.playerCurrentGalaxy];
        if (!galaxy) return;

        solarSystem = galaxy->solarSystems[map_manager.playerCurrentSolarSystem];
        if (!solarSystem) return;

        gfc_vector2d_add(position, solarSystem->pos, gfc_vector2d(100 + 2, 61 - 31));
        map_manager.playerMarker->position = position;
        ui_draw_element(map_manager.playerMarker);
        
    } else if (map_manager.view == UNIVERSE_VIEW) {
        galaxy = world_get_universe()->galaxies[map_manager.playerCurrentGalaxy];
        if (!galaxy) return;

        gfc_vector2d_add(position, galaxy->pos, gfc_vector2d(100 + 5, 61 - 32));
        map_manager.playerMarker->position = position;
        ui_draw_element(map_manager.playerMarker);
    } else {
        if (player != NULL && map_manager.solarSystem == map_manager.playerCurrentSolarSystem && map_manager.galaxy == map_manager.playerCurrentGalaxy) {
            gfc_vector2d_add(position, gfc_vector2d(player->position.x / DISTANCE_SCALE_FACTOR, player->position.y / DISTANCE_SCALE_FACTOR), gfc_vector2d(640 - 15, 360 - 48));
            map_manager.playerMarker->position = position;
            ui_draw_element(map_manager.playerMarker);
        }
        
        if (map_manager.jumpTarget) {
            ui_draw_element(map_manager.jumpTargetSelector);
            ui_draw_element(map_manager.jumpButton);
        }
    }
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

    if (!galaxy || galaxy->numSolarSystems <= 0 ||!galaxy->solarSystems) return;
    if (map_manager.galaxyView) {
        world_map_unload_view_elements(map_manager.galaxyView, map_manager.solarSystems);
        map_manager.baseUI->children = NULL;
        map_manager.baseUI->childCount = 0;
        map_manager.galaxyView = NULL;
    }

    galaxyView = (UIElement**) gfc_allocate_array(sizeof(UIElement*), galaxy->numSolarSystems);
    for (i = 0; i < galaxy->numSolarSystems; i++) {
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
    map_manager.jumpTarget = 0;
}

void world_map_on_click() {
    UIElement* el;
    Uint32 index;
    int i;
    for (i = 0; i < map_manager.baseUI->childCount; i++) {
        el = map_manager.baseUI->children[i];
        if (el && gf2d_mouse_in_rect(el->localBounding)) {
            index = ((ViewIconData*) el->data)->index;
            if (map_manager.view == UNIVERSE_VIEW) {
                world_map_load_solarsystems(world_get_universe()->galaxies[index]);
                map_manager.galaxy = index;
                map_manager.view = GALAXY_VIEW;
            } else if (map_manager.view == GALAXY_VIEW){
                world_map_load_solarsystem(world_get_universe()->galaxies[map_manager.galaxy]->solarSystems[index]);
                map_manager.solarSystem = index;
                map_manager.view = SOLAR_SYSTEM_VIEW;
            }
            return;
        }
    }

    if (map_manager.view == SOLAR_SYSTEM_VIEW && map_manager.jumpTarget) {
        if (gf2d_mouse_in_rect(map_manager.jumpTargetSelector->localBounding)) {
            map_manager.jumpTarget = 0;
        } else if (gf2d_mouse_in_rect(map_manager.jumpButton->localBounding) && player) {
            Galaxy* galaxy = world_get_universe()->galaxies[map_manager.galaxy];
            if (!galaxy) return;

            SolarSystem* solarSystem = galaxy->solarSystems[map_manager.solarSystem];
            if (!solarSystem) return;

            if (player_try_ftl(galaxy, solarSystem, gfc_vector3d((map_manager.jumpTargetSelector->position.x - 640) * DISTANCE_SCALE_FACTOR, (map_manager.jumpTargetSelector->position.y - 360) * DISTANCE_SCALE_FACTOR, 0))) {
                map_manager.jumpTarget = 0;
                map_manager.playerCurrentGalaxy = map_manager.galaxy;
                map_manager.playerCurrentSolarSystem = map_manager.solarSystem;
            }
            ui_close_menu();
        }
    }
}

void world_map_on_right_click() {
    GFC_Vector2D position;
    if (map_manager.view == SOLAR_SYSTEM_VIEW) {
        position = gf2d_mouse_get_position();
        gfc_vector2d_add(map_manager.jumpTargetSelector->position, position, gfc_vector2d(-15, -48));
        map_manager.jumpTarget = 1;
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
            return;
        }
    }

    map_manager.locationSelector->position = gfc_vector2d(-100, -100);
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
    map_manager.baseUI->onRightClick = world_map_on_right_click;
    map_manager.baseUI->onHover = world_map_on_hover;
    map_manager.baseUI->onClose = world_map_on_close;

    map_manager.locationSelector = ui_element_create_simple("images/ui/map/location_selector.png", gfc_vector2d(-9 + 100, -8 + 61));
    map_manager.playerMarker = ui_element_create_simple("images/ui/map/player_marker.png", gfc_vector2d(0,0));
    map_manager.jumpTargetSelector = ui_element_create_simple("images/ui/map/jump_target.png", gfc_vector2d(0, 0));
    map_manager.jumpButton = ui_element_create_simple("images/ui/map/ftl_jump_button.png", gfc_vector2d(550, 580));

    univ = world_get_universe();

    univMap = (UIElement**) gfc_allocate_array(sizeof(UIElement*), univ->numGalaxies);
    for (i = 0; i < univ->numGalaxies; i++) {
        gfc_vector2d_add(position, univ->galaxies[i]->pos, gfc_vector2d(100, 61));
        univMap[i]  = ui_element_create_simple("images/ui/map/galaxy_icon.png", position);
        data = gfc_allocate_array(sizeof(ViewIconData), 1);
        data->index = i;
        univMap[i]->data = data;
    }
    map_manager.universeView = univMap;
    map_manager.baseUI->children = univMap;
    map_manager.baseUI->childCount = univ->numGalaxies;
    map_manager.galaxies = univ->numGalaxies;
    GFC_TextLine name = "openmap";
    register_command_callback(name, world_map_get);
}

void world_map_set_player_location(int galaxyIndex, int solarSystemIndex) {
    map_manager.playerCurrentGalaxy = galaxyIndex;
    map_manager.playerCurrentSolarSystem = solarSystemIndex;
}

struct UIElement_s* world_map_get() {
    return map_manager.baseUI;
}
