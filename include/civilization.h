#ifndef __CIVILIZATION_H__
#define __CIVILIZATION_H__s

#include "gfc_list.h"
#include "gfc_vector.h"

#include "entity.h"
#include "resource.h"

typedef enum CivilType_e {
    CIVIL_TYPE_UNKNOWN,
    CIVIL_TYPE_MINING
} CivilType;

typedef struct CivilTrade_s {
    ResourceAmount give;
    ResourceAmount take;
} CivilTrade;

typedef struct CivilMission_s {
    ResourceAmount goal;
    ResourceAmount reward;
} CivilMission;

typedef struct Civilization_s {
    const char *name;
    CivilType type;
    GFC_List* trades;
    GFC_List* missions;
    const char *model;
    const char *texture;
} Civilization;

typedef struct CivilizationEntityData_s {
    const Civilization *civilization;
} CivilizationEntityData;

void civilization_init();

const Civilization* civilization_get_by_name(const char *name);

Entity* civilization_spawn(GFC_Vector3D pos, const Civilization* civilization);

void civilization_trade_open(const Civilization* civ);

#endif /* __CIVILIZATION_H__ */