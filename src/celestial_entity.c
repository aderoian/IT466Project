#include "simple_logger.h"

#include "gfc_color.h"
#include "gf3d_texture.h"
#include "gf3d_mesh.h"

#include "celestial_generator.h"

#include "celestial_entity.h"

#define DISTANCE_SCALE_FACTOR 120
#define BODY_SCALE_FACTOR 30

Entity* spawn_celestial_entity(CelestialBody* body) {
    Entity* self;
    if (!body) return NULL;
    if (!body->settings) return NULL;

    self = entity_new();
    strcpy(self->name, "celestial");
    self->mesh = generate_celestial_body(body->settings);
    if (!self->mesh) return NULL;
    self->texture = gf3d_texture_load(body->texture);
    self->color = GFC_COLOR_WHITE;
    self->position = gfc_vector3d(body->pos.x * DISTANCE_SCALE_FACTOR, body->pos.y * DISTANCE_SCALE_FACTOR, 0);
    self->rotation = quaternion_create(0, 0, 0, 1);
    self->scale = gfc_vector3d(1, 1, 1);//gfc_vector3d(body->settings->radius * 2 * BODY_SCALE_FACTOR, body->radius * 2 * BODY_SCALE_FACTOR, body->radius * 2 * BODY_SCALE_FACTOR);
    body->entity = self;

    self->physicsBody = physics_body_create();
    self->physicsBody->position = self->position;
    self->physicsBody->mass = body->mass;
    self->physicsBody->invMass = 0; // Planets become "static"
    self->physicsBody->owner = self;
    self->physicsBody->shape.type = FLAG_SPHERE;
    self->physicsBody->shape.Shape.sphere.w = body->settings->radius;
    return self;
}

void asteroid_think(Entity* ent) {
    AsteroidEntityData* data;
    if (!ent) return;

    data = ent->data;
    if (data->health <= 0) {
        // TODO: Resource drop
        ent->think = entity_free;
    }
}

void asteroid_on_collide(const CollisionInfo* info) {
    Entity *asteroid, *other;
    if (!info->b || !info->b->owner || !info->a || !info->a->owner) return;

    if (strcmp(info->a->owner->name, "asteroid") == 0) {
        asteroid = info->a->owner;
        other = info->b->owner;
    } else {
        other = info->a->owner;
        asteroid = info->b->owner;
    }

    if (strcmp(other->name, "celestial") == 0) {
        asteroid->think = entity_free;
        world_get()->numAsteroids--;
    }
}

Entity* spawn_asteroid(GFC_Vector3D position, float size) {
    Entity* self;
    float theta, z, r, speed;
    GFC_Vector3D dir;

    self = entity_new();
    strcpy(self->name, "asteroid");
    self->mesh = gf3d_mesh_load("models/primitives/sphere.obj");
    self->texture = gf3d_texture_load("models/primitives/flatwhite.png");
    self->color = GFC_COLOR_YELLOW;
    self->position = position;
    self->rotation = quaternion_create(0, 0, 0, 1);
    self->scale = gfc_vector3d(size * 2.f, size * 2.f, size * 2.f);
    self->data = (AsteroidEntityData*) gfc_allocate_array(sizeof(AsteroidEntityData), 1);
    ((AsteroidEntityData*) self->data)->health = 10 * size;
    self->think = asteroid_think;

    self->physicsBody = physics_body_create();
    self->physicsBody->position = self->position;
    self->physicsBody->mass = 100;
    self->physicsBody->invMass = 1.f / 100;
    self->physicsBody->owner = self;
    self->physicsBody->shape.type = FLAG_SPHERE;
    self->physicsBody->shape.Shape.sphere.w = size * 2;

    theta = gfc_random() * GFC_2PI;
    z = gfc_random() * 2.0f - 1.0f;
    r = sqrtf(1.0f - z * z);
    dir = gfc_vector3d(r * cosf(theta), z, r * sinf(theta));
    speed = (gfc_random() * 480.f) + 20.f;
    gfc_vector3d_scale(self->physicsBody->velocity, dir, speed);
    return self;
}

Entity* spawn_generated_celestial_entity(Texture *texture, GFC_Vector3D scale) {
    Entity* self;
    if (!texture) return NULL;

    self = entity_new();
    strcpy(self->name, "celestial");
    self->texture = texture;
    self->color = GFC_COLOR_WHITE;
    self->position = gfc_vector3d(0, 0, 0);
    self->rotation = quaternion_create(0, 0, 0, 1);
    self->scale = scale;

    return self;
}