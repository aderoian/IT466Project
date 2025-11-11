#include "simple_logger.h"
#include "gfc_input.h"
#include "gf3d_vgraphics.h"
#include "gf2d_mouse.h"
#include "bullet_entity.h"

#include "player.h"

#define PLAYER_SPEED 1000.0f
#define PLAYER_SENSITIVITY 0.001f
#define PLAYER_ROLL_SENSITIVITY 0.33f
#define PLAYER_ROTATION_CLAMP 50
#define PLAYER_SPEED_CLAMP 200.f
#define PLAYER_MOVEMENT_DRAG 1.50f
#define PLAYER_ROLL_CLAMP 3.5f
#define ROTATION_DT 0.035f

#define clamp(x, low, high) ((x) < (low) ? (low) : ((x) > (high) ? (high) : (x)))

Entity* player = NULL;

void on_collide(const CollisionInfo* info) {
    Entity *player, *other;
    PlayerData* pData;
    GFC_Vector3D vel;
    if (!info->b || !info->b->owner || !info->a || !info->a->owner) return;

    if (strcmp(info->a->owner->name, "player") == 0) {
        player = info->a->owner;
        other = info->b->owner;
        vel = info->aVelocity;
    } else {
        other = info->a->owner;
        player = info->b->owner;
        vel = info->bVelocity;
    }

    pData = (PlayerData*) player->data;
    if (strcmp(other->name, "asteroid") == 0 || strcmp(other->name, "celestial") == 0) {
        pData->ship.durability -= gfc_vector3d_magnitude(vel) * 0.10f;
    }
}

void player_think(Entity* ent) {
    PlayerData* data;
    Ship* ship;
    int i, dir = 0, boost;
    float usableEnergy = 0, usedEnergy = 0, speed = 0, usage, diff;
    if (!ent) return;
    data = (PlayerData*) ent->data;
    ship = &data->ship;

    for (i = 0; i < 3; i++) {
        if (ship->weapons[i].fireCooldown > 0)
            ship->weapons[i].fireCooldown--;
    }

    for (i = 0; i < ship->hull->maxReactors; i++) {
        ship->storedEnergy[i] = fminf(ship->reactors[i]->capacity, ship->storedEnergy[i] + ship->reactors[i]->rechargeSpeed);
        usableEnergy += ship->storedEnergy[i];
    }

    slog("usable energy %f", usableEnergy);

    dir = gfc_input_command_down("thrustforward") ? 1 : (gfc_input_command_down("thrustback") ? -1 : 0);
    boost = gfc_input_command_down("thrustboost");
    if (dir) {
        for (i = 0; i < ship->hull->maxEngines; i++) {
            usage = boost ? ship->engines[i]->boostUsage : ship->engines[i]->usage;
            if (usedEnergy + usage > usableEnergy) continue;
            speed += boost ? ship->engines[i]->boostThrust : ship->engines[i]->thrust;
            usedEnergy += usage;
        }
    }

    slog("thrust used energy %f/%f", usedEnergy, usableEnergy);

    data->deltaSpeed = dir * speed;
    data->deltaMaxSpeed = speed;

    data->weaponsFired = 0;
    for (i = 0; i < 3; i++) {
        if (!ship->weapons[i].weapon) continue;
        if (gf2d_mouse_button_held(i) && ship->weapons[i].fireCooldown <= 0) {
            usage = ship->weapons[i].weapon->depletion;
            if (usedEnergy + usage > usableEnergy) continue;
            data->weaponsFired |= (1 << i);
            usedEnergy += usage;
        }
    }

    slog("weapon used energy %f/%f", usedEnergy, usableEnergy);

    for (i = 0; i < ship->hull->maxReactors; i++) {
        diff = fminf(fminf(ship->storedEnergy[i], usedEnergy), ship->reactors[i]->capacity);
        ship->storedEnergy[i] -= diff;
        usedEnergy -= diff;
    }

    if (gfc_input_command_pressed("upgradereactor")) {
        data->reactorL = (data->reactorL + 1) % 5;
        data->ship.reactors[0] = &g_reactors[data->reactorL];
    }
    if (gfc_input_command_pressed("upgradehull")) {
        data->hullL = (data->hullL + 1) % 5;
        data->ship.hull = &g_hulls[data->hullL];
    }
    if (gfc_input_command_pressed("upgradeengine")) {
        data->engineL = (data->engineL + 1) % 5;
        data->ship.engines[0] = &g_engines[data->engineL];
    }
    if (gfc_input_command_pressed("upgradeweapon")) {
        data->weaponL = (data->weaponL + 1) % 3;
        data->ship.weapons[0].weapon = &g_weapons[data->weaponL];
    }
    if (gfc_input_command_pressed("upgradestorage")) {
        data->ship.storage += 150.f;
    }

}

void player_update(Entity* ent) {
    Quaternion* rot, delta;
    int mdx, mdy, i;
    float half, rollDelta = 0, speedSq;
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
    data->rotVelocity.y =+ clamp(data->rotVelocity.y + rollDelta, -PLAYER_ROLL_CLAMP, PLAYER_ROLL_CLAMP);

    rot = &ent->rotation;
    rotation.x = data->rotVelocity.x * ROTATION_DT;
    rotation.z = data->rotVelocity.z * ROTATION_DT;
    rotation.y = data->rotVelocity.y * 0.015;
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

    speedSq = gfc_vector3d_magnitude_squared(ent->physicsBody->velocity);
    quaternion_rotate_v(&forward, *rot, gfc_vector3d(0,1,0));
    velocity = ent->physicsBody->velocity;
    if (speedSq < 0.0001) {
        gfc_vector3d_clear(ent->physicsBody->velocity);
    } else {
        gfc_vector3d_normalize(&velocity);
        gfc_vector3d_scale(velocity, velocity, fminf(sqrt(speedSq), PLAYER_MOVEMENT_DRAG));
        gfc_vector3d_sub(ent->physicsBody->velocity, ent->physicsBody->velocity, velocity);
    }

    if (speedSq < data->deltaMaxSpeed * data->deltaMaxSpeed) {
        gfc_vector3d_scale(velocity, forward, data->deltaSpeed);
        physics_add_impulse(ent->physicsBody, velocity);
    }

    for (i = 0; i < 3; i++) {
        if (data->weaponsFired & (1 << i)) {
            player_fire_weapon(ent, &data->ship.weapons[i]);
        }
    }
}

void player_free(Entity* ent) {
    player = NULL;
    if (!ent)
        return;

    if (ent->data) free(ent->data);
}

Entity* init_player(GFC_Vector3D position, GFC_Color color) {
    PlayerData* data = NULL;

    Entity* self;
    self = entity_new();
    if (!self) return NULL;

    gfc_line_cpy(self->name, "player");
    self->mesh = gf3d_mesh_load("models/dino/dino.obj");
    self->texture = gf3d_texture_load("models/dino/dino.png");
    if (!self->mesh || !self->texture) {
        slog("ERROR: NO MESH OR TEXTURE");
    }
    self->color = color;
    self->position = position;

    data = gfc_allocate_array(sizeof(PlayerData), 1);
    ship_init_basic(&data->ship);
    self->data = data;
    self->free = player_free;
    self->think = player_think;
    self->update = player_update;

    self->physicsBody = physics_body_create();
    self->physicsBody->position = position;
    self->physicsBody->mass = 25;
    self->physicsBody->invMass = 1.f / 25.f;
    self->physicsBody->owner = self;
    self->physicsBody->shape.type = FLAG_SPHERE;
    self->physicsBody->shape.Shape.sphere.w = 7;
    self->physicsBody->collisionCallback = on_collide;

    player = self;
    return self;
}

int player_fire_weapon(Entity* ent, WeaponSlot* slot) {
    if (!ent || !slot || !slot->weapon) return 0;

    slot->fireCooldown = 1500.f / slot->weapon->fireRate;
    slot->weapon->fire(slot->weapon, ent);
    return 1;
}
