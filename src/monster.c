#include "simple_logger.h"
#include "gfc_vector.h"
#include "gfc_color.h"
#include "gf3d_texture.h"
#include "gf3d_mesh.h"

#include "physics.h"
#include "entity.h"

#include "monster.h"

Entity* monster_spawn(GFC_Vector3D position, GFC_Color color) {
    Entity* self;
    self = entity_new();
    if (!self) return NULL;

    // populate monster
    gfc_line_cpy(self->name, "placeholder obstacle");
    self->mesh = gf3d_mesh_load("models/dino/dino.obj");
    self->texture = gf3d_texture_load("models/dino/dino.png");
    self->color = color;
    self->position = position;
    self->rotation = quaternion_create(0, 0, 0, 1);

    self->physicsBody = physics_body_create();
    self->physicsBody->flags |= FLAG_NOT_COLLIDABLE;
    self->physicsBody->owner = self;
    self->physicsBody->position = position;

    return self;
}

void monster_think(Entity* ent) {
    if (!ent) return;

    if (gfc_vector3d_is_zero(ent->velocity)) {
        ent->velocity.z = gfc_random();
    }

    if (ent->position.z < 0) {
        ent->velocity.z = gfc_random();
    } else if (ent->position.z > 50) {
        ent->velocity.z = -(gfc_random());
    }

}

void monster_update(Entity* ent) {
    if (!ent) return;
    gfc_vector3d_add(ent->position, ent->velocity, ent->position);
    gfc_vector3d_add(ent->rotation, gfc_vector3d(0, 0, gfc_random()/10.0f), ent->rotation);
}
