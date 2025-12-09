#include "simple_logger.h"
#include "simple_json.h"

#include "def.h"
#include "entity.h"

#include "building.h"

typedef struct BuildingList_s {
    Building *buildings;
    int count;
} BuildingList;

BuildingList g_buildingList = {0};

void building_load_production_from_def(SJson* json, BuildingProduction *prod);

void building_init() {
    DefinitionData *def, *bListDef, *bDef, *bProdList, *bProd;
    Building* building;
    int i, j;
    def = def_load("defs/buildings.def");
    if (!def) {
        slog("Failed to load buildings.def\n");
        return;
    }

    bListDef = def_data_get_array(def, "buildings");
    if (!bListDef) {
        slog("No buildings found in buildings.def\n");
        return;
    }

    def_data_array_get_count(bListDef, &g_buildingList.count);
    g_buildingList.buildings = malloc(sizeof(Building) * g_buildingList.count);
    for (i = 0; i < g_buildingList.count; i++) {
        building = &g_buildingList.buildings[i];
        bDef = def_data_array_get_nth(bListDef, i);
        building->name = strdup(def_data_get_string(bDef, "name"));
        building->mesh = strdup(def_data_get_string(bDef, "model"));
        building->texture = strdup(def_data_get_string(bDef, "texture"));
        
        bProdList = def_data_get_array(bDef, "products");
        def_data_array_get_count(bProdList, &building->productCount);
        building->production = gfc_allocate_array(sizeof(BuildingProduction), building->productCount);
        for (j = 0; j < building->productCount; j++) {
            bProd = def_data_array_get_nth(bProdList, j);
            building_load_production_from_def(bProd, &building->production[j]);
        }
    }
}

const Building* building_get_by_name(const char *name) {
    int i;
    for (i = 0; i < g_buildingList.count; i++) {
        if (strcmp(g_buildingList.buildings[i].name, name) == 0)
            return &g_buildingList.buildings[i];
    }
    return NULL;
}

void building_load_production_from_def(SJson* json, BuildingProduction *prod) {
    if (!json || !prod) return;
    resource_amount_from_config(def_data_get_obj(json, "production"), &prod->production);
    def_data_get_float(json, "duration", &prod->productionDuration);
}

Entity* building_spawn_entity(Entity* planet, const Building* building, GFC_Vector3D surfacePosition) {
    GFC_Vector3D dir;
    Entity* self;
    if (!planet || !building) return NULL;

    self = entity_new();
    strcpy(self->name, "building");
    self->mesh = gf3d_mesh_load(building->mesh);
    if (!self->mesh) return NULL;
    self->position = surfacePosition;
    self->texture = gf3d_texture_load(building->texture);
    self->rotation = quaternion_create(0, 0, 0, 1);
    self->color = GFC_COLOR_WHITE;
    self->scale = gfc_vector3d(3, 3, 3);
    return self;
}