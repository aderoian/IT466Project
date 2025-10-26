#include "gfc_input.h"

#include "camera_entity.h"

void camera_entity_think(Entity* ent) {


}
void camera_entity_free(Entity* ent) {
    if (!ent) return;
    if (ent->data) free(ent->data);
}

Entity* camera_entity_spawn(GFC_Vector3D pos, Entity* target) {
    Entity* entity;
    CameraEntityData* data;
    GFC_Vector3D dir = {0};

    entity = entity_new();
    data = gfc_allocate_array(sizeof(CameraEntityData), 1);
    camera_data.data = data;
    camera_data.target = target;
    gfc_vector3d_copy(camera_data.targetPos, target->position);

    entity->think = camera_entity_think;
    entity->free = camera_entity_free;
    entity->position = pos;

    gfc_vector3d_sub(dir, target->position, pos);
    gfc_vector3d_normalize(&dir);
    gfc_vector3d_add(target->postion, pos, dir);
    gf3d_camera_look_at(target->position, &self->position);

    data->target = target;
    data->followAngle = GFC_PI;
    data->followHeight = 15;
    data->followDist = 30;

    return entity;
}
