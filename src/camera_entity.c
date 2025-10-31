#include "simple_logger.h"
#include "gfc_input.h"

#include "camera_entity.h"

typedef struct {
    Entity* target;
    Quaternion rotation;
    float followBehind;
    float followHeight;
} CameraEntityData;

void camera_entity_think(Entity* ent) {
    CameraEntityData* data;
    GFC_Vector3D forward, up,cameraPos, followBehindVec, followHeightVec;
    if (!ent || !ent->data) return;

    data = (CameraEntityData*) ent->data;
    if (!data->target) {
        slog("error: camera has no target entity");
        return;
    }

    // Extract forward and up axis and create a new camera position
    quaternion_rotate_v(&forward, data->target->rotation, gfc_vector3d(0,1,0));
    quaternion_rotate_v(&up, data->target->rotation, gfc_vector3d(0,0,1));
    gfc_vector3d_scale(followBehindVec, forward, -data->followBehind);
    gfc_vector3d_scale(followHeightVec, up, data->followHeight);
    gfc_vector3d_copy(cameraPos, data->target->position);
    gfc_vector3d_add(cameraPos, cameraPos, followBehindVec);
    gfc_vector3d_add(ent->position, cameraPos, followHeightVec);
    data->rotation = data->target->rotation;
}

void camera_entity_update(Entity* ent) {
    if (!ent || !ent->data) return;
    gf3d_camera_set_position(ent->position);
    gf3d_camera_set_rotation_q(((CameraEntityData*) ent->data)->rotation);
}

void camera_entity_free(Entity* ent) {
    if (!ent) return;
    if (ent->data) free(ent->data);
}

Entity* camera_entity_spawn(GFC_Vector3D pos, Entity* target, float followBehind, float followHeight) {
    CameraEntityData* data;
    Entity* entity;

    entity = entity_new();
    data = gfc_allocate_array(sizeof(CameraEntityData), 1);
    data->target = target;
    data->followBehind = followBehind;
    data->followHeight = followHeight;
    quaternion_identity(&data->rotation);
    entity->data = data;

    entity->think = camera_entity_think;
    entity->update = camera_entity_update;
    entity->free = camera_entity_free;
    entity->position = pos;

    return entity;
}
