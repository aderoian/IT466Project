# ifndef CAMERA_ENTITY_H
# define CAMERA_ENTITY_H

#include "gfc_vector.h"
#include "gf3d_camera.h"

#include "entity.h"

typedef struct {
    Entity* target;
    GFC_Vector3D targetPos;
    float followDist;
    float followHeight;
    float followAngle;
} CameraEntityData;

Entity* camera_entity_spawn(GFC_Vector3D pos, Entity* target);

# endif CAMERA_ENTITY_H
