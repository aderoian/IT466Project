#include "simple_logger.h"

#include "gf2d_font.h"
#include "player.h"
#include "ui.h"
#include "overlay.h"

static UIElement* overlay;
static UIElement* shipStats;
static UIElement* weaponStats;

void overlay_draw() {
    char buffer[50];
    GFC_Vector2D pos;
    PlayerData* data;
    float energy = 0;
    int i;

    gf2d_sprite_draw_image(shipStats->sprite, shipStats->position);
    gf2d_sprite_draw_image(weaponStats->sprite, weaponStats->position);

    if (!player) return;
    data = (PlayerData*) player->data;

    // Draw Ship stats
    gfc_vector2d_add(pos, shipStats->position, gfc_vector2d(124, 10));
    sprintf(buffer, "%.0f", gfc_vector3d_magnitude(player->velocity));
    gf2d_font_draw_line_tag(buffer, FT_Normal, GFC_COLOR_WHITE, pos);

    gfc_vector2d_add(pos, pos, gfc_vector2d(0, 44));
    sprintf(buffer, "%.2f percent", (data->ship.durability / data->ship.hull->durability) * 100);
    gf2d_font_draw_line_tag(buffer, FT_Normal, GFC_COLOR_WHITE, pos);

    for (i = 0 ; i < data->ship.hull->maxReactors; i++)
        energy += data->ship.storedEnergy[i];

    gfc_vector2d_add(pos, pos, gfc_vector2d(0, 44));
    sprintf(buffer, "%.0f", energy);
    gf2d_font_draw_line_tag(buffer, FT_Normal, GFC_COLOR_WHITE, pos);

    gfc_vector2d_add(pos, pos, gfc_vector2d(0, 44));
    sprintf(buffer, "%.2f", data->ship.storage);
    gf2d_font_draw_line_tag(buffer, FT_Normal, GFC_COLOR_WHITE, pos);

    // Draw Weapon Stats
    gfc_vector2d_add(pos, weaponStats->position, gfc_vector2d(160, 15));
    gf2d_font_draw_line_tag(data->ship.weapons[0].weapon ? data->ship.weapons[0].weapon->name : "None", FT_Small, GFC_COLOR_WHITE, pos);

    gfc_vector2d_add(pos, pos, gfc_vector2d(0, 44));
    gf2d_font_draw_line_tag(data->ship.weapons[1].weapon ? data->ship.weapons[1].weapon->name : "None", FT_Small, GFC_COLOR_WHITE, pos);

    gfc_vector2d_add(pos, pos, gfc_vector2d(0, 44));
    gf2d_font_draw_line_tag(data->ship.weapons[2].weapon ? data->ship.weapons[2].weapon->name : "None", FT_Small, GFC_COLOR_WHITE, pos);
}

void overlay_update(float delta) {

}

void overlay_init() {
    overlay = gfc_allocate_array(sizeof(UIElement), 1);
    overlay->update = overlay_update;
    overlay->draw = overlay_draw;
    ui_set_overlay(overlay);
    shipStats = ui_element_create_simple("images/ui/overlay/ship_stats.png", gfc_vector2d(0, 520));
    weaponStats = ui_element_create_simple("images/ui/overlay/weapon_stats.png", gfc_vector2d(960, 572));
    atexit(overlay_close);
}

void overlay_close() {
    free(overlay);
    free(shipStats);
    free(weaponStats);
}
