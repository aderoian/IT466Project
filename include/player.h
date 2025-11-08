# ifndef PLAYER_H
# define PLAYER_H

#include "gfc_vector.h"
#include "gfc_color.h"

#include "entity.h"
#include "weapon.h"

typedef struct {
    GFC_Vector3D rotVelocity;
    Weapon* weapon;
    int fireCooldown;
} PlayerData;

/**
 * @brief Setup the player entity
 */
Entity* init_player(GFC_Vector3D position, GFC_Color color);

int player_try_fire_weapon(Entity* ent);

int player_fire_weapon(Entity* ent);

# endif
