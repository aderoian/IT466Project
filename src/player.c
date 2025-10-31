#include "simple_logger.h"
#include "gfc_input.h"

#include "player.h"

#define PLAYER_SPEED 10.0f

void player_update(Entity* ent) {
    if (!ent) return;
    Quaternion* rot, delta;
    float half, dx = 0, dy = 0, dz = 0;

    rot = &ent->rotation;
    if (gfc_input_command_down("walkforward")) {
        dx += 0.11f;
    }
    if (gfc_input_command_down("walkback")) {
        dx -= 0.11f;
    }
    if (gfc_input_command_down("walkleft")) {
        dz += 0.11f;
    }
    if (gfc_input_command_down("walkright")) {
        dz -= 0.11f;
    }
    if (gfc_input_command_down("rollleft")) {
        dy -= 0.11f;
    }
    if (gfc_input_command_down("rollright")) {
        dy += 0.11f;
    }

    half = dx * 0.5f;
    delta = quaternion_create(sinf(half), 0, 0, cosf(half));
    quaternion_multiply_q(rot, *rot, delta);

    half = dy * 0.5f;
    delta = quaternion_create(0, sinf(half), 0, cosf(half));
    quaternion_multiply_q(rot, *rot, delta);

    half = dz * 0.5f;
    delta = quaternion_create(0, 0, sinf(half), cosf(half));
    quaternion_multiply_q(rot, *rot, delta);
    quaternion_normalize(rot);
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

    self->data = data;
    self->free = player_free;
    self->update = player_update;

    return self;
}
