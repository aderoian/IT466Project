#include "simple_logger.h"

#include "gfc_color.h"
#include "gf3d_texture.h"
#include "gf3d_mesh.h"

#include "celestial_entity.h"

#define DISTANCE_SCALE_FACTOR 20
#define BODY_SCALE_FACTOR 5

Entity* spawn_celestial_entity(CelestialBody* body) {
    Entity* self;
    if (!body) return NULL;

    self = entity_new();
    self->mesh = gf3d_mesh_load("models/primitives/sphere.obj");
    self->texture = gf3d_texture_load("models/primitives/flatwhite.png");
    self->color = GFC_COLOR_WHITE;
    self->position = gfc_vector3d(body->pos.x * DISTANCE_SCALE_FACTOR, body->pos.y * DISTANCE_SCALE_FACTOR, 0);
    self->rotation = quaternion_create(0, 0, 0, 1);
    self->scale = gfc_vector3d(body->radius * 2 * BODY_SCALE_FACTOR, body->radius * 2 * BODY_SCALE_FACTOR, body->radius * 2 * BODY_SCALE_FACTOR);

    self->physicsBody = physics_body_create();
    self->physicsBody->position = self->position;
    self->physicsBody->mass = body->mass;
    self->physicsBody->invMass = 0; // Planets become "static"
    self->physicsBody->owner = self;
    self->physicsBody->shape.type = FLAG_SPHERE;
    self->physicsBody->shape.Shape.sphere.w = body->radius * 2 * BODY_SCALE_FACTOR;
    return self;
}
