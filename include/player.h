# ifndef PLAYER_H
# define PLAYER_H

#include "gfc_vector.h"
#include "gfc_color.h"

#include "entity.h"
#include "weapon.h"
#include "ship.h"
#include "world.h"
#include "civilization.h"

typedef struct {
    GFC_Vector3D rotVelocity;
    Ship ship;
    float deltaSpeed;
    float deltaMaxSpeed;
    int weaponsFired;

    int reactorL;
    int hullL;
    int engineL;
    int weaponL;

    const Civilization *civilContact;
} PlayerData;

extern Entity* player;

/**
 * @brief Setup the player entity
 */
Entity* init_player(GFC_Vector3D position, GFC_Color color);

int player_fire_weapon(Entity* ent, WeaponSlot* slot);

int player_try_ftl(Galaxy* galaxy, SolarSystem* targetSolarSystem, GFC_Vector3D targetPos);

# endif
