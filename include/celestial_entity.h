#ifndef __CELESTIAL_ENTITY_H__
#define __CELESTIAL_ENTITY_H__

#include "entity.h"
#include "world.h"

struct ShapeSettings_S;

typedef struct CelestialEntityData_s {
    const struct ShapeSettings_S* settings;
} CelestialEntityData;

typedef struct AsteroidEntityData_s {
    float health;
} AsteroidEntityData;

Entity* spawn_celestial_entity(CelestialBody* body);

Entity* spawn_asteroid(GFC_Vector3D position, float size);

Entity* spawn_generated_celestial_entity(const struct ShapeSettings_S *settings, Texture *texture, GFC_Vector3D scale);

#endif // __CELESTIAL_ENTITY_H__
