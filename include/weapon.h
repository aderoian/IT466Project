#ifndef __WEAPON_H__
#define __WEAPON_H__

#include "gf3d_texture.h"
#include "gf3d_mesh.h"
#include "entity.h"
#include "gfc_text.h"

typedef enum AmmoType_s {
    ROUND,
    PLASMA
} AmmoType;

typedef struct AmmoResource_s {
    char* model;
    char* texture;
} AmmoResource;

typedef struct Weapon_s {
    GFC_TextLine name;
    AmmoType ammoType;
    AmmoResource* ammoResource;
    int depletion;
    int reloadDelay;
    int fireRate;
    int damage;
    int lifetime;
    int bulletSpeed;
    void (*fire) (struct Weapon_s* weapon, Entity* shooter);
} Weapon;

extern Weapon g_weapons[];

void fire_lazer_cannon(Weapon* weapon, Entity* shooter);
void fire_dual_lazer_cannon(Weapon* weapon, Entity* shooter);
void fire_quad_lazer_cannon(Weapon* weapon, Entity* shooter);
void fire_plasma_beam(Weapon* weapon, Entity* shooter);

#endif // __WEAPON_H__
