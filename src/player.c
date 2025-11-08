#include "simple_logger.h"
#include "gfc_input.h"
#include "gf3d_vgraphics.h"
#include "gf2d_mouse.h"
#include "bullet_entity.h"

#include "player.h"

#define PLAYER_SPEED 1000.0f
#define PLAYER_SENSITIVITY 0.001f
#define PLAYER_ROLL_SENSITIVITY 0.75f
#define PLAYER_ROTATION_CLAMP 50
#define PLAYER_SPEED_CLAMP 1000.f
#define PLAYER_MOVEMENT_DRAG 2.25f
#define PLAYER_ROLL_CLAMP 5
#define ROTATION_DT 0.035f

#define clamp(x, low, high) ((x) < (low) ? (low) : ((x) > (high) ? (high) : (x)))

void on_collide(const CollisionInfo* info) {
    //slog("collided with %f %f %f at %f %f %f", info->b->position.x, info->b->position.y, info->b->position.z, info->contactPoint.x, info->contactPoint.y, info->contactPoint.z);
}
void player_think(Entity* ent) {
    PlayerData* data;
    if (!ent) return;
    data = (PlayerData*) ent->data;

    if (data->fireCooldown > 0) data->fireCooldown--;
}

void player_update(Entity* ent) {
    Quaternion* rot, delta;
    int mdx, mdy;
    float half, rollDelta = 0, speed;
    GFC_Vector3D forward, velocity, rotation;
    PlayerData* data;
    if (!ent) return;

    data = (PlayerData*) ent->data;

    SDL_GetRelativeMouseState(&mdx, &mdy);
    data->rotVelocity.z += clamp(mdx, -PLAYER_ROTATION_CLAMP, PLAYER_ROTATION_CLAMP) * PLAYER_SENSITIVITY;
    data->rotVelocity.x += clamp(mdy, -PLAYER_ROTATION_CLAMP, PLAYER_ROTATION_CLAMP) * PLAYER_SENSITIVITY;

    if (gfc_input_command_down("rollleft"))
        rollDelta -= PLAYER_ROLL_SENSITIVITY;
    if (gfc_input_command_down("rollright"))
        rollDelta += PLAYER_ROLL_SENSITIVITY;
    data->rotVelocity.y =+ clamp(rollDelta, -PLAYER_ROLL_CLAMP, PLAYER_ROLL_CLAMP);

    rot = &ent->rotation;
    gfc_vector3d_scale(rotation, data->rotVelocity, ROTATION_DT);
    gfc_vector3d_sub(data->rotVelocity, data->rotVelocity, rotation);

    half = -rotation.x * 0.5f;
    delta = quaternion_create(sinf(half), 0, 0, cosf(half));
    quaternion_multiply_q(rot, *rot, delta);

    half = rotation.y * 0.5f;
    delta = quaternion_create(0, sinf(half), 0, cosf(half));
    quaternion_multiply_q(rot, *rot, delta);

    half = -rotation.z * 0.5f;
    delta = quaternion_create(0, 0, sinf(half), cosf(half));
    quaternion_multiply_q(rot, *rot, delta);
    quaternion_normalize(rot);

    speed = gfc_vector3d_magnitude_squared(ent->physicsBody->velocity);

    velocity = ent->physicsBody->velocity;
    if (speed < 0.0001) {
        gfc_vector3d_clear(ent->physicsBody->velocity);
    } else {
        gfc_vector3d_normalize(&velocity);
        gfc_vector3d_scale(velocity, velocity, fminf(speed, PLAYER_MOVEMENT_DRAG));
        gfc_vector3d_sub(ent->physicsBody->velocity, ent->physicsBody->velocity, velocity);
    }

    slog("vel %f %f %f, %f", ent->physicsBody->velocity.x, ent->physicsBody->velocity.y, ent->physicsBody->velocity.z, speed);
    if (gfc_input_command_down("thrustforward") && speed < PLAYER_SPEED_CLAMP * 2) {
        slog("adding speed");
        quaternion_rotate_v(&forward, *rot, gfc_vector3d(0,1,0));
        gfc_vector3d_scale(velocity, forward, PLAYER_SPEED);
        physics_add_impulse(ent->physicsBody, velocity);
    }
    if (gfc_input_command_down("thrustback") && speed > -PLAYER_SPEED_CLAMP * 2) {
        quaternion_rotate_v(&forward, *rot, gfc_vector3d(0,-1,0));
        gfc_vector3d_scale(velocity, forward, PLAYER_SPEED);
        physics_add_impulse(ent->physicsBody, velocity);
    }

    if (gfc_input_command_down("fire")) {
        player_try_fire_weapon(ent);
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

    data = gfc_allocate_array(sizeof(PlayerData), 1);
    self->data = data;
    self->free = player_free;
    self->think = player_think;
    self->update = player_update;

    self->physicsBody = physics_body_create();
    self->physicsBody->position = position;
    self->physicsBody->mass = 100;
    self->physicsBody->invMass = 1.f / 100.f;
    self->physicsBody->owner = self;
    self->physicsBody->shape.type = FLAG_SPHERE;
    self->physicsBody->shape.Shape.sphere.w = 7;
    self->physicsBody->collisionCallback = on_collide;

    data->weapon = &g_weapons[0];

    return self;
}

int player_try_fire_weapon(Entity* ent) {
    PlayerData* data;
    if (!ent) return 0;
    data = (PlayerData*) ent->data;

    if (data->fireCooldown > 0) return 0;
    return player_fire_weapon(ent);
}

int player_fire_weapon(Entity* ent) {
    PlayerData* data;
    if (!ent) return 0;
    data = (PlayerData*) ent->data;

    data->fireCooldown = 1500.f / data->weapon->fireRate;
    data->weapon->fire(data->weapon, ent);
    return 1;
}
