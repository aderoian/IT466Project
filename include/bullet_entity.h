#ifndef __BULLET_ENTITY__
#define __BULLET_ENTITY__

#include "gfc_vector.h"
#include "entity.h"

typedef struct BulletEntityData_s {
    int lifetime;
    Entity* owner;
} BulletEntityData;

Entity* bullet_spawn(Entity* owner, GFC_Vector3D position, GFC_Vector3D velocity, char* model, char* texture, int lifetime);

void bullet_free(Entity* entity);

#endif //__BULLET_ENTITY__
