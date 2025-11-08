#include "simple_logger.h"

#include "gfc_color.h"

#include "bullet_entity.h"

void bullet_think(Entity* entity) {
    if (!entity);

    if (((BulletEntityData*)entity->data)->lifetime-- <= 0) {
        entity->think = entity_free;
    }
}

Entity* bullet_spawn(Entity* owner, GFC_Vector3D position, GFC_Vector3D velocity, char* model, char* texture, int lifetime) {
    Entity* self;
    if (!owner || !model || !texture) return NULL;

    self = entity_new();
    if (!self) return NULL;

    self->mesh = gf3d_mesh_load(model);
    self->texture = gf3d_texture_load(texture);
    self->color = GFC_COLOR_WHITE;
    self->position = position;

    self->data = (BulletEntityData*) gfc_allocate_array(sizeof(BulletEntityData), 1);
    ((BulletEntityData*)self->data)->lifetime = lifetime;
    ((BulletEntityData*)self->data)->owner = owner;

    self->physicsBody = physics_body_create();
    self->physicsBody->position = self->position;
    self->physicsBody->mass = 0.5f;
    self->physicsBody->invMass = 1/self->physicsBody->mass;
    self->physicsBody->owner = self;

    physics_add_impulse(self->physicsBody, velocity);
    return self;
}

void bullet_free(Entity* entity) {
    if (!entity) return;
    if (entity->data) free(entity->data);
}
