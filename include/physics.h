#ifndef __PHYSICS_H__
#define __PHYSICS_H__

#include "gfc_vector.h"

#define FLAG_NOT_COLLIDABLE 0x01
#define FLAG_NO_COLLISION_RESOLUTION 0x02

#define SOLVER_ITERATIONS 3
#define POSITIONAL_CORRECTION_PERCENT 0.8f
#define POSITIONAL_CORRECTION_SLOP 0.01f

#define FLAG_AABB 0x01
#define FLAG_SPHERE 0x02

typedef GFC_Vector3D AABBShape[2];
typedef GFC_Vector4D SphereShape;

typedef struct {
    Uint8 type;
    union {
        AABBShape aabb;
        SphereShape sphere;
    } Shape;
} CollisionShape;

struct Entity_s;
struct CollisionInfo_s;

typedef struct PhysicsBody_s {
    Uint8 _inuse;
    GFC_Vector3D position;
    GFC_Vector3D velocity;
    GFC_Vector3D forceAccumulator;
    float mass;
    float invMass;
    Uint8 flags;
    CollisionShape shape;

    struct Entity_s* owner;
    void (*collisionCallback)(const struct CollisionInfo_s *info);
} PhysicsBody;

typedef struct CollisionInfo_s {
    PhysicsBody* a;
    PhysicsBody* b;
    GFC_Vector3D aVelocity;
    GFC_Vector3D bVelocity;
    float penetration;
    GFC_Vector3D contactPoint;
    GFC_Vector3D contactNormal;
} CollisionInfo;

void physics_init(int nbodies);
void physics_close();

PhysicsBody* physics_body_create();
void physics_body_free(PhysicsBody* body);

void physics_step(float deltaTime);

void physics_add_impulse(PhysicsBody* body, GFC_Vector3D impulse);
void physics_add_force(PhysicsBody* body, GFC_Vector3D force);

# endif // __PHYSICS_H__
