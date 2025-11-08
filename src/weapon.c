#include "simple_logger.h"

#include "weapon.h"
#include "bullet_entity.h"

static AmmoResource ammoResources[] = {
    {
        "models/weapon/ammo/ammo_lazer.obj",
        "models/primitives/flatwhite.png"
    }
};

Weapon g_weapons[] = {
    {
        "Lazer Cannon",         // Name
        PLASMA,                 // Ammo Type
        &ammoResources[0],      // Ammo Resource
        100000,                 // Mag Size
        10,                     // Depletion
        100,                    // Reload Delay
        100,                    // Fire Rate
        fire_lazer_cannon       // Fire Function
    }
};

void fire_lazer_cannon(Weapon* weapon, Entity* shooter) {
    Entity* bullet;
    GFC_Vector3D velocity;
    if (!weapon || !shooter) return;

    quaternion_rotate_v(&velocity, shooter->rotation, gfc_vector3d(0,1,0));
    gfc_vector3d_scale(velocity, velocity, 1000);

    bullet = bullet_spawn(shooter, shooter->position, velocity, weapon->ammoResource->model, weapon->ammoResource->texture, 1000);
    if (bullet) quaternion_copy(&bullet->rotation, shooter->rotation);
}
