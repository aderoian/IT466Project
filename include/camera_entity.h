# ifndef CAMERA_ENTITY_H
# define CAMERA_ENTITY_H

#include "gfc_vector.h"
#include "gf3d_camera.h"

#include "entity.h"
#include "quaternion.h"

extern Entity* cameraEntity;

/**
 * @brief Spawns the camera entity which is reponsible for moving and rotating the game camera.
 *
 * @param pos Starting position of the camera.
 * @param target The entity the camera will follow.
 * @param followBehind The distance (magnitude) the camera should follow behind the target entity.
 * @param followHeight The distance (magnitude) the camera will be offset above the target entity.
 *
 * @return The spawned camera entity, NULL if error.
 */
Entity* camera_entity_spawn(GFC_Vector3D pos, Entity* target, float followBehind, float followHeight);

void camera_entity_set_target(Entity* cameraEntity, Entity* target);

Entity* camera_entity_editor_spawn(GFC_Vector3D pos);

#endif
