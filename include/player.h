# ifndef PLAYER_H
# define PLAYER_H

#include "gfc_vector.h"
#include "gfc_color.h"

#include "entity.h"
#include "weapon.h"
#include "ship.h"
#include "world.h"

struct ResourceAmount_s;
struct Civilization_s;

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

    const struct Civilization_s *civilContact;
} PlayerData;

extern Entity* player;

/**
 * @brief Setup the player entity
 */
Entity* init_player(GFC_Vector3D position, GFC_Color color);

int player_fire_weapon(Entity* ent, WeaponSlot* slot);

int player_try_ftl(Entity* player, Galaxy* galaxy, SolarSystem* targetSolarSystem, GFC_Vector3D targetPos);

int player_try_take_resource(Entity* player, const struct ResourceAmount_s* resAmount);
void player_give_resource(Entity* player, const struct ResourceAmount_s* resAmount);

# endif
