#ifndef __WORLD_H__
#define __WORLD_H__

#include "gfc_list.h"
#include "gfc_text.h"

#include "entity.h"

struct SolarSystem_s;

typedef struct World_s {
    struct SolarSystem_s* solarSystem;
    GFC_List* asteroids;
    int numAsteroids;
} World;

typedef enum {
    PLANET,
    ASTEROID,
    MOON,
    SUN
} CelestialBodyType;

typedef struct CelestialBody_s {
    CelestialBodyType type;
    GFC_TextLine name;
    Entity* entity;
    char texture[50];
    GFC_Vector2D pos;
    float mass;
    float radius;
} CelestialBody;

typedef struct SolarSystem_s {
    GFC_TextLine name;
    GFC_Vector2D pos;
    CelestialBody** celestialBodies;
    int numBodies;
} SolarSystem;

typedef struct Galaxy_s {
    GFC_TextLine name;
    GFC_Vector2D pos;
    SolarSystem** solarSystems;
    int numSolarSystems;
} Galaxy;

typedef struct Universe_s {
    GFC_TextLine name;
    Galaxy** galaxies;
    int numGalaxies;
} Universe;

void world_init();
void world_close();
void world_update();

void world_load_universe(Universe* universe);
void world_save_universe(Universe* universe);

World* world_get();
Universe* world_get_universe();
SolarSystem* world_get_target_solarSystem();
void world_set_target_solarSystem(SolarSystem* solarSystem);

void world_generate_universe(Universe *out, int width, int height);

#endif // __WORLD_H__
