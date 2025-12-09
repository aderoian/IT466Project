# ifndef PLAYER_H
# define PLAYER_H

#include "gfc_vector.h"
#include "gfc_color.h"

#include "entity.h"
#include "weapon.h"
#include "ship.h"
#include "world.h"

struct ResourceAmount_s;
struct Resource_s;
struct Civilization_s;
struct CivilMission_s;
struct Building_S;

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
    GFC_List *civilMissions;

    struct ResourceAmount_s* inventory;
    Uint32 invSize;
    Uint32 invCap;
} PlayerData;

extern Entity* player;

/**
 * @brief Setup the player entity
 */
Entity* init_player(GFC_Vector3D position, GFC_Color color);

int player_fire_weapon(Entity* ent, WeaponSlot* slot);

int player_try_ftl(Entity* player, Galaxy* galaxy, SolarSystem* targetSolarSystem, GFC_Vector3D targetPos);


int player_start_mission(Entity* player, struct CivilMission_s *mission);
int player_try_end_mission(Entity* player, struct CivilMission_s *mission, Uint8 cancel);

int player_has_resource_of(Entity* player, const struct Resource_s* resource, int amount);
int player_try_take_resource(Entity* player, const struct ResourceAmount_s* resAmount);
int player_try_give_resource(Entity* player, const struct ResourceAmount_s* resAmount);

int player_try_init_build(Entity* player);
int player_try_build(Entity* player, const struct Building_S *building, Entity *planet, GFC_Vector3D pos);

# endif
