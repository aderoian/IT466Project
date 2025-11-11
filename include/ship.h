#ifndef __SHIP_H__
#define __SHIP_H__

#include "gfc_list.h"

struct Weapon_s;

typedef struct WeaponSlot_s {
   struct Weapon_s* weapon;
   int fireCooldown;
   //int fireUnits; // Energy or ammo currently in this slot
} WeaponSlot;

typedef struct ShipReactor_s {
    float capacity;
    float rechargeSpeed;
} ShipReactor;

typedef struct ShipHull_s {
    float durability;
    int maxReactors;
    int maxEngines;
} ShipHull;

typedef struct ShipEngine_s {
    float thrust;
    float usage;
    float boostThrust;
    float boostUsage;
} ShipEngine;

typedef struct Ship_s {
    ShipHull* hull;
    ShipReactor** reactors;
    float* storedEnergy;
    ShipEngine** engines;
    WeaponSlot weapons[3]; // Primary, secondary, special
    float durability;
    float storage;
} Ship;

extern ShipReactor g_reactors[];
extern ShipHull    g_hulls[];
extern ShipEngine  g_engines[];

void ship_init_basic(Ship* out);

#endif // __SHIP_H__
