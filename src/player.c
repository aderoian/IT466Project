#include "simple_logger.h"
#include "gfc_input.h"

#include "player.h"

#define PLAYER_SPEED 10.0f

void player_update(Entity* ent) {
    if (!ent) return;
    if (gfc_input_command_down("walkforward")) {
        slog("walkforward");
        ent->position.y += PLAYER_SPEED;
    } else if (gfc_input_command_down("walkback")) {
        slog("walkback");
        ent->position.y -= PLAYER_SPEED;
    } else if (gfc_input_command_down("walkleft")) {
        slog("walkleft");
        ent->position.x -= PLAYER_SPEED;
    } else if (gfc_input_command_down("walkright")) {
        slog("walkright");
        ent->position.x += PLAYER_SPEED;
    }
    slog("%f, %f, %f", gfc_vector3d_to_slog(ent->position));
}

void player_free(Entity* ent) {
    if (!ent)
        return;

    if (ent->data) free(ent->data);
}

Entity* init_player(GFC_Vector3D position, GFC_Color color) {
    PlayerData* data = NULL;

    Entity* self;
    self = entity_new();
    if (!self) return NULL;

    gfc_line_cpy(self->name, "test player");
    self->mesh = gf3d_mesh_load("models/dino/dino.obj");
    self->texture = gf3d_texture_load("models/dino/dino.png");
    if (!self->mesh || !self->texture) {
        slog("ERROR: NO MESH OR TEXTURE");
    }
    self->color = color;
    self->position = position;
    self->rotation = gfc_vector3d(0, 0, 90);

    self->data = data;
    self->free = player_free;
    self->update = player_update;


    slog("%f, %f, %f", gfc_vector3d_to_slog(self->position));
    return self;
}
