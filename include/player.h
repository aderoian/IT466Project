# ifndef PLAYER_H
# define PLAYER_H

#include "gfc_vector.h"
#include "gfc_color.h"

#include "entity.h"

typedef struct {
    GFC_Vector3D rotVelocity;
} PlayerData;

/**
 * @brief Setup the player entity
 */
Entity* init_player(GFC_Vector3D position, GFC_Color color);

# endif
