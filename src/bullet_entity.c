#include "simple_logger.h"

#include "gfc_color.h"

#include "celestial_entity.h"
#include "bullet_entity.h"
#include "player.h"

void bullet_think(Entity* entity) {
    if (!entity);

    if (((BulletEntityData*)entity->data)->lifetime-- <= 0) {
        entity->think = entity_free;
    }
}

void bullet_on_collide(const CollisionInfo* info) {
    Entity *bullet, *other;
    BulletEntityData* bData;
    AsteroidEntityData* aData;
    if (!info->b || !info->b->owner || !info->a || !info->a->owner) return;

    if (strcmp(info->a->owner->name, "bullet") == 0) {
        bullet = info->a->owner;
        other = info->b->owner;
    } else {
        other = info->a->owner;
        bullet = info->b->owner;
    }

    bData = bullet->data;
    aData = other->data;
    if (strcmp(other->name, "asteroid") == 0) {
        aData->health -= bData->weapon->damage;
        if (strcmp(bData->owner->name, "player") == 0) {
            player_destroy_asteroid(bData->owner);
        }
    }

    bullet->think = entity_free;
}

Entity* bullet_spawn(Entity* owner, const Weapon* weapon, GFC_Vector3D position, GFC_Vector3D velocity) {
    Entity* self;
    BulletEntityData* data;
    if (!owner || !weapon) return NULL;

    self = entity_new();
    if (!self) return NULL;

    strcpy(self->name, "bullet");
    self->mesh = gf3d_mesh_load(weapon->ammoResource->model);
    self->texture = gf3d_texture_load(weapon->ammoResource->texture);
    self->color = GFC_COLOR_WHITE;
    self->position = position;

    data = (BulletEntityData*) gfc_allocate_array(sizeof(BulletEntityData), 1);
    data->lifetime = weapon->lifetime;
    data->owner = owner;
    data->weapon = weapon;
    self->data = data;
    self->think = bullet_think;

    self->physicsBody = physics_body_create();
    self->physicsBody->position = self->position;
    self->physicsBody->mass = 0.5f;
    self->physicsBody->invMass = 1/self->physicsBody->mass;
    self->physicsBody->owner = self;
    self->physicsBody->shape.type = FLAG_SPHERE;
    self->physicsBody->shape.Shape.sphere.w = 1.f;
    self->physicsBody->collisionCallback = bullet_on_collide;

    physics_add_impulse(self->physicsBody, velocity);
    return self;
}

void bullet_free(Entity* entity) {
    if (!entity) return;
    if (entity->data) free(entity->data);
}
