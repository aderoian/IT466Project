#ifndef __BUILDING_H__
#define __BUILDING_H__

#include "gfc_vector.h"

#include "resource.h"

struct Entity_s;

typedef struct BuildingProduction_S {
    ResourceAmount production;
    ResourceAmount productionCost;
    int productionDuration;
} BuildingProduction;

typedef struct Building_S {
    char* name;
    int productCount;
    BuildingProduction *production;
    int costAmount;
    ResourceAmount *cost;
    char *mesh;
    char *texture;
} Building;

void building_init();

const Building* building_get_by_name(const char *name);

struct Entity_s* building_spawn_entity(struct Entity_s* planet, const Building* building, GFC_Vector3D surfacePosition);

void building_menu_open(Entity *planet, GFC_Vector3D pos);

#endif // __BUILDING_H__