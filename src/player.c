#include "simple_logger.h"
#include "gfc_input.h"

#include "player.h"

#define PLAYER_SPEED 500.0f

void player_update(Entity* ent) {
    if (!ent) return;
    Quaternion* rot, delta;
    float half, dx = 0, dy = 0, dz = 0;
    GFC_Vector3D forward, velocity;

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

    if (gfc_input_command_down("thrustforward")) {
        quaternion_rotate_v(&forward, *rot, gfc_vector3d(0,1,0));
        gfc_vector3d_scale(velocity, forward, PLAYER_SPEED);
        physics_add_impulse(ent->physicsBody, velocity);
    }
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

    self->physicsBody = physics_body_create();
    self->physicsBody->mass = 100;
    self->physicsBody->invMass = 1.f / 100.f;
    self->physicsBody->owner = self;

    return self;
}
