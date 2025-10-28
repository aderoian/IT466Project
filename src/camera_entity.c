#include "simple_logger.h"
#include "gfc_input.h"

#include "camera_entity.h"

typedef struct {
    Quaternion rotation;
} CameraEntityData;

void camera_entity_think(Entity* ent) {
    // Use a quat to track overall rotation
    // somehow convert quat -> matrix for rendering
    // look_at calculates a delta rotation and applies it to look at specific targets
    //      possibilty quat->matrix and compare with target then convert back to quat
    Quaternion* rot, delta;
    float half, dx = 0, dy = 0, dz = 0;

    rot = &((CameraEntityData*) ent->data)->rotation;
    if (gfc_input_command_down("walkforward")) {
        dx += 0.11f;
    }
    if (gfc_input_command_down("walkback")) {
        dx -= 0.11f;
    }
    if (gfc_input_command_down("walkleft")) {
        dz -= 0.11f;
    }
    if (gfc_input_command_down("walkright")) {
        dz += 0.11f;
    }
    if (gfc_input_command_down("rollleft")) {
        dy -= 0.11f;
    }
    if (gfc_input_command_down("rollright")) {
        dy += 0.11f;
    }

    half = dx * 0.5f;
    delta = quaternion_create(sinf(half), 0, 0, cosf(half));
    quaternion_multiply_q(rot, *rot, delta);

    half = dy * 0.5f;
    delta = quaternion_create(0, sinf(half), 0, cosf(half));
    quaternion_multiply_q(rot, *rot, delta);

    half = dz * 0.5f;
    delta = quaternion_create(0, 0, sinf(half), cosf(half));
    quaternion_multiply_q(rot, *rot, delta);
    quaternion_normalize(rot);
}

void camera_entity_update(Entity* ent) {
    GFC_Vector3D eulerRot;
    quaternion_euler_angles_from(&eulerRot, ((CameraEntityData*) ent->data)->rotation);
    gf3d_camera_set_rotation(eulerRot);
}

void camera_entity_free(Entity* ent) {
    if (!ent) return;
    if (ent->data) free(ent->data);
}

Entity* camera_entity_spawn(GFC_Vector3D pos) {
    CameraEntityData* data;
    Entity* entity;

    entity = entity_new();
    data = gfc_allocate_array(sizeof(CameraEntityData), 1);
    entity->data = data;

    entity->think = camera_entity_think;
    entity->update = camera_entity_update;
    entity->free = camera_entity_free;
    entity->position = pos;
    quaternion_identity(&((CameraEntityData*) entity->data)->rotation);

//     forward = target.pos - me.pos
//     norm(forward)
//     right = forward x entity.up
//
//
//     mat4 q;
//     gfc_matrix_from_vectors_q(q, {0,0,0}, quat(right, roll), {1,1,1});
//     gfc_matrix4_v_multiply()

    return entity;
}
