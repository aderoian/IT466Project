#include <SDL.h>
#include "simple_logger.h"
#include "gfc_list.h"

#include "physics.h"
#include "entity.h"

#define clamp(x, low, high) ((x) < (low) ? (low) : ((x) > (high) ? (high) : (x)))

typedef struct {
    PhysicsBody* bodies;
    Uint32 count;
} PhysicsBodyManager;

extern int __DEBUG;
PhysicsBodyManager body_manager = {0};

GFC_List* collisionBuffer = NULL;

void physics_integrate(float deltaTime);
void physics_sync();
void physics_detect_collision();
int physics_check_aabbaabb(CollisionInfo *info, PhysicsBody* a, PhysicsBody* b);
int physics_check_aabbsphere(CollisionInfo *info, PhysicsBody* a, PhysicsBody* b);
int physics_check_sphereaabb(CollisionInfo *info, PhysicsBody* a, PhysicsBody* b);
int physics_check_spheresphere(CollisionInfo *info, PhysicsBody* a, PhysicsBody* b);
void physics_resolve_collisions();
void physics_resolve_collision(const CollisionInfo *info);
void physics_resolve_collision_position(const CollisionInfo *info);
void physics_process_events();

void physics_init(int nbodies) {
    if (nbodies <= 0) {
        slog("Cannot init physics body with zero entities");
        return;
    }

    body_manager.bodies = (PhysicsBody *)gfc_allocate_array(sizeof(PhysicsBody),nbodies);
    body_manager.count = nbodies;

    collisionBuffer = gfc_list_new_size(nbodies);

    if(__DEBUG)slog("physics body manager initialized");
    atexit(physics_close);
}

void physics_close() {
    int i;
    for (i = 0; i < body_manager.count;i++)
    {
        if (body_manager.bodies[i]._inuse > 0) {
            physics_body_free(&body_manager.bodies[i]);
        }
    }
    if (body_manager.bodies)
    {
        free(body_manager.bodies);
    }
    memset(&body_manager,0,sizeof(PhysicsBodyManager));

    gfc_list_delete(collisionBuffer);

    if(__DEBUG)slog("physics body manager closed");
}

PhysicsBody* physics_body_create() {
    int i;
    for (i = 0; i < body_manager.count; i++) {
        if (body_manager.bodies[i]._inuse > 0)continue;
        body_manager.bodies[i]._inuse = 1;
        return &body_manager.bodies[i];
    }
    return NULL;
}

void physics_body_free(PhysicsBody* body) {
    if (!body) return;
    memset(body,0,sizeof(PhysicsBody));
}

void physics_step(float deltaTime) {
    int i;
    CollisionInfo* info;

    physics_integrate(deltaTime);
    physics_detect_collision();
    physics_resolve_collisions();
    physics_sync();
    physics_process_events();

    for (i = 0; i < gfc_list_get_count(collisionBuffer); i++) {
        info = (CollisionInfo*) gfc_list_get_nth(collisionBuffer, i);
        if (!info) continue;
        free(info);
    }

    gfc_list_clear(collisionBuffer);
}

void physics_integrate(float deltaTime) {
    int i;
    GFC_Vector3D vel;
    for (i = 0; i < body_manager.count; i++) {
        if (body_manager.bodies[i]._inuse <= 0)continue;
        gfc_vector3d_scale(vel, body_manager.bodies[i].forceAccumulator, body_manager.bodies[i].invMass);
        gfc_vector3d_scale(vel, vel, deltaTime);
        gfc_vector3d_add(body_manager.bodies[i].velocity, body_manager.bodies[i].velocity, vel);
        gfc_vector3d_scale(vel, body_manager.bodies[i].velocity, deltaTime);
		gfc_vector3d_add(body_manager.bodies[i].position, vel, body_manager.bodies[i].position);
    }
}

void physics_sync() {
    int i;
    for (i = 0; i < body_manager.count; i++) {
        if (body_manager.bodies[i]._inuse <= 0)continue;

        if (!body_manager.bodies[i].owner) continue;
        body_manager.bodies[i].owner->position = body_manager.bodies[i].position;
        body_manager.bodies[i].owner->velocity = body_manager.bodies[i].velocity;
    }
}

void physics_detect_collision() {
    int i, j;
    PhysicsBody *a, *b;
    CollisionInfo* info;
    int collision = 0;

    info = gfc_allocate_array(sizeof(CollisionInfo), 1);
    for (i = 0; i < body_manager.count; i++) {
        if (body_manager.bodies[i]._inuse <= 0)continue;
        if (body_manager.bodies[i].flags & FLAG_NOT_COLLIDABLE) continue;

        for (j = i + 1; j < body_manager.count; j++) {
            if (body_manager.bodies[j]._inuse <= 0)continue;
            if (body_manager.bodies[j].flags & FLAG_NOT_COLLIDABLE) continue;
            a = &body_manager.bodies[i];
            b = &body_manager.bodies[j];

            if (a->shape.type & FLAG_AABB && b->shape.type & FLAG_AABB) {
                collision = physics_check_aabbaabb(info, a,b);
            } else if (a->shape.type & FLAG_AABB && b->shape.type & FLAG_SPHERE) {
                collision = physics_check_aabbsphere(info, a, b);
            } else if (a->shape.type & FLAG_SPHERE && b->shape.type & FLAG_AABB) {
                collision = physics_check_sphereaabb(info, a, b);
            } else if (a->shape.type & FLAG_SPHERE && b->shape.type & FLAG_SPHERE) {
                collision = physics_check_spheresphere(info, a, b);
            }

            if (collision) {
                info->a = a;
                info->b = b;
                info->aVelocity = a->velocity;
                info->bVelocity = b->velocity;

                gfc_list_append(collisionBuffer, info);
                info = gfc_allocate_array(sizeof(CollisionInfo), 1);
            }
        }
    }

    if (!collision) free(info);
}

int physics_check_aabbaabb(CollisionInfo* info, PhysicsBody* a, PhysicsBody* b) {
    AABBShape aWorld = {0}, bWorld = {0};
    GFC_Vector3D contactMin, contactMax;

    gfc_vector3d_add(aWorld[0], a->position, a->shape.Shape.aabb[0]);
    gfc_vector3d_add(aWorld[1], a->position, a->shape.Shape.aabb[1]);
    gfc_vector3d_add(bWorld[0], b->position, b->shape.Shape.aabb[0]);
    gfc_vector3d_add(bWorld[1], b->position, b->shape.Shape.aabb[1]);

    float xOverlap = fminf(aWorld[1].x, bWorld[1].x) - fmaxf(aWorld[0].x, bWorld[0].x);
    float yOverlap = fminf(aWorld[1].y, bWorld[1].y) - fmaxf(aWorld[0].y, bWorld[0].y);
    float zOverlap = fminf(aWorld[1].z, bWorld[1].z) - fmaxf(aWorld[0].z, bWorld[0].z);

    if (xOverlap <= 0 || yOverlap <= 0 || zOverlap <= 0)
        return 0; // No collision

    info = (CollisionInfo *)gfc_allocate_array(sizeof(CollisionInfo),1);

    // Find smallest overlap axis, compute normal and penetration depth
    info->penetration = xOverlap;
    gfc_vector3d_copy(info->contactNormal, gfc_vector3d(1, 0, 0));
    if (aWorld[1].x > bWorld[1].x) {
        gfc_vector3d_negate(info->contactNormal, info->contactNormal);
    }
    if (yOverlap < info->penetration) {
        info->penetration = yOverlap;
        gfc_vector3d_copy(info->contactNormal, gfc_vector3d(0, 1, 0));
        if (aWorld[1].y > bWorld[1].y) {
            gfc_vector3d_negate(info->contactNormal, info->contactNormal);
        }
    }
    if (zOverlap < info->penetration) {
        info->penetration = zOverlap;
        gfc_vector3d_copy(info->contactNormal, gfc_vector3d(0, 0, 1));
        if (aWorld[1].z > bWorld[1].z) {
            gfc_vector3d_negate(info->contactNormal, info->contactNormal);
        }
    }

    // Contact point (approximation)
    contactMin = gfc_vector3d(
        fmaxf(aWorld[0].x, bWorld[0].x),
        fmaxf(aWorld[0].y, bWorld[0].y),
        fmaxf(aWorld[0].z, bWorld[0].z)
    );
    contactMax = gfc_vector3d(
        fminf(aWorld[1].x, bWorld[1].x),
        fminf(aWorld[1].y, bWorld[1].y),
        fminf(aWorld[1].z, bWorld[1].z)
    );

    gfc_vector3d_add(info->contactPoint, contactMin, contactMax);
    gfc_vector3d_scale(info->contactPoint, info->contactPoint, 0.5f);
    return 1;
}

int physics_check_aabbsphere(CollisionInfo *info, PhysicsBody* a, PhysicsBody* b) {
	AABBShape aWorld = {0};
    SphereShape bWorld = {0};
	GFC_Vector3D diff;
	float distSq, radiusSq;

    gfc_vector3d_add(aWorld[0], a->position, a->shape.Shape.aabb[0]);
    gfc_vector3d_add(aWorld[1], a->position, a->shape.Shape.aabb[1]);
    gfc_vector3d_add(bWorld, b->shape.Shape.sphere, b->position);
	info->contactPoint.x = clamp(bWorld.x, aWorld[0].x, aWorld[1].x);
	info->contactPoint.y = clamp(bWorld.y, aWorld[0].y, aWorld[1].y);
	info->contactPoint.z = clamp(bWorld.z, aWorld[0].z, aWorld[1].z);

	gfc_vector3d_sub(diff, bWorld, info->contactPoint);
    gfc_vector3d_copy(info->contactNormal, diff);
    gfc_vector3d_normalize(&info->contactNormal);
	distSq = gfc_vector3d_dot_product(diff, diff);
	radiusSq = b->shape.Shape.sphere.w * b->shape.Shape.sphere.w;
	if (distSq > radiusSq) {
		return 0;
	}

	info->penetration = radiusSq - sqrtf(distSq);
	return 1;
}

int physics_check_sphereaabb(CollisionInfo* info, PhysicsBody* a, PhysicsBody* b) {
    SphereShape aWorld = {0};
	AABBShape bWorld = {0};
	GFC_Vector3D diff;
	float distSq, radiusSq;

    gfc_vector3d_add(bWorld[0], b->position, b->shape.Shape.aabb[0]);
    gfc_vector3d_add(bWorld[1], b->position, b->shape.Shape.aabb[1]);
    gfc_vector3d_add(aWorld, a->shape.Shape.sphere, a->position);
	info->contactPoint.x = clamp(aWorld.x, bWorld[0].x, bWorld[1].x);
	info->contactPoint.y = clamp(aWorld.y, bWorld[0].y, bWorld[1].y);
	info->contactPoint.z = clamp(aWorld.z, bWorld[0].z, bWorld[1].z);

	gfc_vector3d_sub(diff, info->contactPoint, aWorld);
    gfc_vector3d_copy(info->contactNormal, diff);
    gfc_vector3d_normalize(&info->contactNormal);
	distSq = gfc_vector3d_dot_product(diff, diff);
	radiusSq = a->shape.Shape.sphere.w * a->shape.Shape.sphere.w;
	if (distSq > radiusSq) {
		return 0;
	}

	info->penetration = radiusSq - sqrtf(distSq);
	return 1;
}

int physics_check_spheresphere(CollisionInfo *info, PhysicsBody* a, PhysicsBody* b) {
    SphereShape aWorld = {0}, bWorld = {0};
    GFC_Vector3D diff;
	gfc_vector3d_add(aWorld, a->position, a->shape.Shape.sphere);
	gfc_vector3d_add(bWorld, b->position, b->shape.Shape.sphere);

	gfc_vector3d_sub(diff, bWorld, aWorld);
	float distSq = gfc_vector3d_dot_product(diff, diff);
	float radiusSum = a->shape.Shape.sphere.w + b->shape.Shape.sphere.w;
	if (distSq > radiusSum * radiusSum) {
		return 0;
	}

	gfc_vector3d_copy(info->contactNormal, diff);
    gfc_vector3d_normalize(&info->contactNormal);
	info->penetration = radiusSum - sqrtf(distSq);
    gfc_vector3d_scale(info->contactPoint, info->contactNormal, a->shape.Shape.sphere.w);
	gfc_vector3d_add(info->contactPoint, info->contactPoint, aWorld);
	return 1;
}

void physics_resolve_collisions() {
    int i, j;
    CollisionInfo* info;
    for (j = 0; j < SOLVER_ITERATIONS; j++) {
        for (i = 0; i < gfc_list_get_count(collisionBuffer); i++) {
            info = (CollisionInfo*) gfc_list_get_nth(collisionBuffer, i);
            if (!info) continue;
            if (!(info->a->flags & FLAG_NO_COLLISION_RESOLUTION || info->b->flags & FLAG_NO_COLLISION_RESOLUTION))
                physics_resolve_collision(info);
        }
    }

    for (i = 0; i < gfc_list_get_count(collisionBuffer); i++) {
            info = (CollisionInfo*) gfc_list_get_nth(collisionBuffer, i);
            if (!info) continue;
            if (!(info->a->flags & FLAG_NO_COLLISION_RESOLUTION || info->b->flags & FLAG_NO_COLLISION_RESOLUTION))
                physics_resolve_collision_position(info);
    }
}

void physics_resolve_collision(const CollisionInfo *info) {
    GFC_Vector3D tmp = {0}, rv = {0}, impulse = {0};
    float velAlongNormal;
    PhysicsBody *a = info->a, *b = info->b;

	gfc_vector3d_sub(rv, b->velocity, b->velocity);
	velAlongNormal = gfc_vector3d_dot_product(rv, info->contactPoint);
	if (velAlongNormal > -0.001f) {
		return;
	}

	gfc_vector3d_scale(impulse, info->contactNormal, -velAlongNormal / (a->invMass + b->invMass));
	gfc_vector3d_scale(tmp, impulse, a->invMass);
	gfc_vector3d_sub(a->velocity, a->velocity, tmp);
	gfc_vector3d_scale(tmp, impulse, b->invMass);
	gfc_vector3d_add(b->velocity, b->velocity, tmp);
}

void physics_resolve_collision_position(const CollisionInfo *info) {
    GFC_Vector3D tmp = {0}, correction = {0};
    PhysicsBody *a = info->a, *b = info->b;

	gfc_vector3d_scale(correction, info->contactNormal, fmaxf(info->penetration - POSITIONAL_CORRECTION_SLOP, 0.0f) * POSITIONAL_CORRECTION_PERCENT);
	gfc_vector3d_scale(tmp, correction, a->invMass / (a->invMass + b->invMass));
	gfc_vector3d_sub(a->position, a->position, tmp);
    gfc_vector3d_scale(tmp, correction, b->invMass / (a->invMass + b->invMass));
	gfc_vector3d_add(b->position, b->position, tmp);
}

void physics_process_events() {
    int i;
    CollisionInfo* info;
    for (i = 0; i < gfc_list_get_count(collisionBuffer); i++) {
        info = (CollisionInfo*) gfc_list_get_nth(collisionBuffer, i);
        if (!info) continue;
        if (info->a->collisionCallback) info->a->collisionCallback(info);
        if (info->b->collisionCallback) info->b->collisionCallback(info);
    }
}

void physics_add_impulse(PhysicsBody* body, GFC_Vector3D impulse) {
    GFC_Vector3D deltaV;
    if (!body) return;
    gfc_vector3d_scale(deltaV, impulse, body->invMass);
	gfc_vector3d_add(body->velocity, body->velocity, deltaV);
}

void physics_add_force(PhysicsBody* body, GFC_Vector3D force) {
    if (!body) return;
    gfc_vector3d_add(body->forceAccumulator, body->forceAccumulator, force);
}
