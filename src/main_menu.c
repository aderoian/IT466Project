#include "gf2d_mouse.h"

#include "ui.h"
#include "world.h"
#include "player.h"
#include "world_map.h"
#include "overlay.h"
#include "camera_entity.h"

#include "main_menu.h"

typedef struct MainMenuManager_s {
    UIElement* baseUI;
    UIElement* playButton;    
} MainMenuManager;

static MainMenuManager menu_manager = {0};

void startGameplay() {
    Entity* player;
    GFC_Vector3D playerPos = {0,0,50};
    
    world_init();

    player = init_player(playerPos, GFC_COLOR_WHITE);
    camera_entity_set_target(cameraEntity, player);

    world_generate_universe(world_get_universe(), 1080, 580);
    world_set_target_solarSystem(world_get_universe()->galaxies[0]->solarSystems[0]);

    world_map_load();
    overlay_init();

    world_map_set_player_location(0, 0);
}

void main_menu_on_click() {
    if (gf2d_mouse_in_rect(menu_manager.playButton->localBounding)) {
        startGameplay();
        ui_close_menu();
    }
}

void main_menu_load() {
    menu_manager.baseUI = ui_element_create_simple("images/ui/main_menu/background.png", gfc_vector2d(418,67));
    menu_manager.baseUI->children = &menu_manager.playButton;
    menu_manager.baseUI->childCount = 1;
    menu_manager.baseUI->onClick = main_menu_on_click;
    menu_manager.playButton = ui_element_create_simple("images/ui/main_menu/play_button.png", gfc_vector2d(543,235));
}
struct UIElement_s* main_menu_get() {
    return menu_manager.baseUI;
}