#ifndef __BULLET_ENTITY__
#define __BULLET_ENTITY__

#include "gfc_vector.h"
#include "weapon.h"
#include "entity.h"

typedef struct BulletEntityData_s {
    int lifetime;
    Entity* owner;
    const Weapon* weapon;
} BulletEntityData;

Entity* bullet_spawn(Entity* owner, const Weapon* weapon, GFC_Vector3D position, GFC_Vector3D velocity);

void bullet_free(Entity* entity);

#endif //__BULLET_ENTITY__
