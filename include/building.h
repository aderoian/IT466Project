#ifndef __BUILDING_H__
#define __BUILDING_H__

#include "gfc_vector.h"

#include "resource.h"

struct Entity_s;

typedef struct BuildingProduction_S {
    ResourceAmount production;
    float productionDuration;
} BuildingProduction;

typedef struct Building_S {
    char* name;
    int productCount;
    BuildingProduction* production;
    char *mesh;
    char *texture;
} Building;

void building_init();

const Building* building_get_by_name(const char *name);

struct Entity_s* building_spawn_entity(struct Entity_s* planet, const Building* building, GFC_Vector3D surfacePosition);

#endif // __BUILDING_H__