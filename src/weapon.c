#include "simple_logger.h"

#include "gfc_audio.h"
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
        "Lazer",                // Name
        PLASMA,                 // Ammo Type
        &ammoResources[0],      // Ammo Resource
        "sounds/weapons/lazer_fire_1.wav", // Sound File
        10,                     // Depletion
        100,                    // Reload Delay
        100,                    // Fire Rate
        10,                     // Damage
        100,                    // Lifetime Duration
        1000,                   // Bullet Speed
        fire_lazer_cannon       // Fire Function
    },
    {
        "Dual Lazer",           // Name
        PLASMA,                 // Ammo Type
        &ammoResources[0],      // Ammo Resource
        "sounds/weapons/lazer_fire_1.wav", // Sound File
        20,                     // Depletion
        100,                    // Reload Delay
        200,                    // Fire Rate
        20,                     // Damage
        100,                    // Lifetime Duration
        1000,                   // Bullet Speed
        fire_dual_lazer_cannon  // Fire Function
    },
    {
        "Quad Lazer",           // Name
        PLASMA,                 // Ammo Type
        &ammoResources[0],      // Ammo Resource
        "sounds/weapons/lazer_fire_1.wav", // Sound File
        40,                     // Depletion
        100,                    // Reload Delay
        400,                    // Fire Rate
        40,                     // Damage
        100,                    // Lifetime Duration
        1000,                   // Bullet Speed
        fire_quad_lazer_cannon  // Fire Function
    },
    {
        "Plasma Beam",          // Name
        PLASMA,                 // Ammo Type
        &ammoResources[0],      // Ammo Resource
        "sounds/weapons/lazer_fire_1.wav", // Sound File
        100,                    // Depletion
        100,                    // Reload Delay
        100,                    // Fire Rate
        150,                    // Damage
        1000,                   // Lifetime Duration
        1000,                   // Bullet Speed
        fire_plasma_beam        // Fire Function
    }
};

void fire_lazer_cannon(Weapon* weapon, Entity* shooter) {
    Entity* bullet;
    GFC_Vector3D velocity;
    GFC_Sound* sound;
    if (!weapon || !shooter) return;

    quaternion_rotate_v(&velocity, shooter->rotation, gfc_vector3d(0,1,0));
    gfc_vector3d_scale(velocity, velocity, weapon->bulletSpeed);
    gfc_vector3d_add(velocity, velocity, shooter->velocity);

    bullet = bullet_spawn(shooter, weapon, shooter->position, velocity);
    if (bullet) quaternion_copy(&bullet->rotation, shooter->rotation);

    sound = gfc_sound_load(weapon->soundFile, 1.0f, 0);
    if (sound) {
        gfc_sound_play_to_group(sound, 0, 1.0f, "world");
    }
}

void fire_dual_lazer_cannon(Weapon* weapon, Entity* shooter) {
    Entity* bullet;
    GFC_Vector3D velocity, right, pos1, pos2;
    GFC_Sound* sound;
    if (!weapon || !shooter) return;

    quaternion_rotate_v(&velocity, shooter->rotation, gfc_vector3d(0,1,0));
    quaternion_rotate_v(&right, shooter->rotation, gfc_vector3d(1,0,0));
    gfc_vector3d_scale(velocity, velocity, weapon->bulletSpeed);
    gfc_vector3d_add(velocity, velocity, shooter->velocity);

    gfc_vector3d_scale(pos1, right, 15);
    gfc_vector3d_scale(pos2, right, -15);

    gfc_vector3d_add(pos1, pos1, shooter->position);
    gfc_vector3d_add(pos2, pos2, shooter->position);

    bullet = bullet_spawn(shooter, weapon, pos1, velocity);
    if (bullet) quaternion_copy(&bullet->rotation, shooter->rotation);
    bullet = bullet_spawn(shooter, weapon, pos2, velocity);
    if (bullet) quaternion_copy(&bullet->rotation, shooter->rotation);

    sound = gfc_sound_load(weapon->soundFile, 1.0f, 0);
    if (sound) {
        gfc_sound_play(sound, 0, 1.0f, -1);
        gfc_sound_free(sound);
    }
}

void fire_quad_lazer_cannon(Weapon* weapon, Entity* shooter) {
    Entity* bullet;
    GFC_Vector3D velocity, up, right, pos1, pos2, pos3, pos4, tmp;
    GFC_Sound* sound;
    if (!weapon || !shooter) return;

    quaternion_rotate_v(&velocity, shooter->rotation, gfc_vector3d(0,1,0));
    quaternion_rotate_v(&right, shooter->rotation, gfc_vector3d(1,0,0));
    quaternion_rotate_v(&up, shooter->rotation, gfc_vector3d(0,0,1));
    gfc_vector3d_scale(velocity, velocity, weapon->bulletSpeed);
    gfc_vector3d_add(velocity, velocity, shooter->velocity);

    gfc_vector3d_scale(pos1, right, 15);
    gfc_vector3d_scale(pos2, right, -15);
    gfc_vector3d_scale(pos3, right, 15);
    gfc_vector3d_scale(pos4, right, -15);
    gfc_vector3d_scale(tmp, up, 15);
    gfc_vector3d_add(pos1, pos1, tmp);
    gfc_vector3d_scale(tmp, up, 15);
    gfc_vector3d_add(pos2, pos2, tmp);
    gfc_vector3d_scale(tmp, up, -15);
    gfc_vector3d_add(pos3, pos3, tmp);
    gfc_vector3d_scale(tmp, up, -15);
    gfc_vector3d_add(pos4, pos4, tmp);

    gfc_vector3d_add(pos1, pos1, shooter->position);
    gfc_vector3d_add(pos2, pos2, shooter->position);
    gfc_vector3d_add(pos3, pos3, shooter->position);
    gfc_vector3d_add(pos4, pos4, shooter->position);

    bullet = bullet_spawn(shooter, weapon, pos1, velocity);
    if (bullet) quaternion_copy(&bullet->rotation, shooter->rotation);
    bullet = bullet_spawn(shooter, weapon, pos2, velocity);
    if (bullet) quaternion_copy(&bullet->rotation, shooter->rotation);
    bullet = bullet_spawn(shooter, weapon, pos3, velocity);
    if (bullet) quaternion_copy(&bullet->rotation, shooter->rotation);
    bullet = bullet_spawn(shooter, weapon, pos4, velocity);
    if (bullet) quaternion_copy(&bullet->rotation, shooter->rotation);

    sound = gfc_sound_load(weapon->soundFile, 1.0f, 0);
    if (sound) {
        gfc_sound_play(sound, 0, 1.0f, -1);
        gfc_sound_free(sound);
    }
}

void fire_plasma_beam(Weapon* weapon, Entity* shooter) {

}
