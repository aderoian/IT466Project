#include "simple_logger.h"
#include "gfc_input.h"

#include "camera_entity.h"

Entity* cameraEntity = NULL;

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

void camera_entity_editor_think(Entity* ent) {
    GFC_Vector3D forward, right, delta = {0}, tmp, rotation = {0};
    float speed = 25.0f, half;
    Quaternion *rot, deltaQ;
    if (!ent) return;

    // get forward and right vectors
    rot = &ent->rotation;
    quaternion_rotate_v(&forward, ent->rotation, gfc_vector3d(0,1,0));
    quaternion_rotate_v(&right, ent->rotation, gfc_vector3d(1,0,0));

    if (gfc_input_command_down("camera_sprint")) speed *= 3;
    if (gfc_input_command_down("camera_forward")) {
        gfc_vector3d_scale(tmp, forward, speed);
        gfc_vector3d_add(delta, delta, tmp);
    }
    if (gfc_input_command_down("camera_back")) {
        gfc_vector3d_scale(tmp, forward, -speed);
        gfc_vector3d_add(delta, delta, tmp);
    }
    if (gfc_input_command_down("camera_left")) {
        gfc_vector3d_scale(tmp, right, -speed);
        gfc_vector3d_add(delta, delta, tmp);
    }
    if (gfc_input_command_down("camera_right")) {
        gfc_vector3d_scale(tmp, right, speed);
        gfc_vector3d_add(delta, delta, tmp);
    }

    gfc_vector3d_add(ent->position, ent->position, delta);

    if(gfc_input_command_down("camera_rot_left")) {
        rotation.z -= 0.05f;
    }
    if(gfc_input_command_down("camera_rot_right")) {
        rotation.z += 0.05f;
    }
    if(gfc_input_command_down("camera_rot_up")) {
        rotation.x -= 0.05f;
    }
    if(gfc_input_command_down("camera_rot_down")) {
        rotation.x += 0.05f;
    }

    half = -rotation.x * 0.5f;
    deltaQ = quaternion_create(sinf(half), 0, 0, cosf(half));
    quaternion_multiply_q(rot, *rot, deltaQ);

    half = -rotation.z * 0.5f;
    deltaQ = quaternion_create(0, 0, sinf(half), cosf(half));
    quaternion_multiply_q(rot, *rot, deltaQ);
    quaternion_normalize(rot);
}

void camera_entity_editor_update(Entity* ent) {
    if (!ent) return;
    gf3d_camera_set_position(ent->position);
    gf3d_camera_set_rotation_q(ent->rotation);
}

void camera_entity_free(Entity* ent) {
    if (!ent) return;
    if (ent->data) free(ent->data);
}

Entity* camera_entity_spawn(GFC_Vector3D pos, Entity* target, float followBehind, float followHeight) {
    CameraEntityData* data;
    Entity* entity;

    entity = entity_new();
    if (!entity) {
        slog("could not create camera entity");
        return NULL;
    }
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

    cameraEntity = entity;
    return entity;
}

void camera_entity_set_target(Entity* cameraEntity, Entity* target) {
    CameraEntityData* data;
    if (!cameraEntity || !cameraEntity->data) return;
    data = (CameraEntityData*) cameraEntity->data;
    data->target = target;
}

Entity* camera_entity_editor_spawn(GFC_Vector3D pos) {
    Entity* entity;
    entity = entity_new();
    if (!entity) {
        slog("could not create camera entity");
        return NULL;
    }

    entity->think = camera_entity_editor_think;
    entity->update = camera_entity_editor_update;
    entity->position = pos;

    cameraEntity = entity;
    return entity;
}
