#ifndef __CELESTIAL_ENTITY_H__
#define __CELESTIAL_ENTITY_H__

#include "entity.h"
#include "world.h"

typedef struct AsteroidEntityData_s {
    float health;
} AsteroidEntityData;

Entity* spawn_celestial_entity(CelestialBody* body);

Entity* spawn_asteroid(GFC_Vector3D position, float size);

Entity* spawn_generated_celestial_entity(Mesh *mesh, Texture *texture, GFC_Vector3D scale);

#endif // __CELESTIAL_ENTITY_H__
