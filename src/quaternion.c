#include "quaternion.h"

Quaternion quaternion_create(float x, float y, float z, float w) {
    return gfc_vector4d(x, y, z, w);
}

void quaternion_copy(Quaternion* dest, Quaternion source) {
    gfc_vector4d_copy((*dest), source);
}

void quaternion_identity(Quaternion* q) {
    q->x = 0;
    q->y = 0;
    q->z = 0;
    q->w = 1;
}

void quaternion_add(Quaternion* dest, Quaternion a, Quaternion b) {
    dest->x = a.x + b.x;
    dest->y = a.y + b.y;
    dest->z = a.z + b.z;
    dest->w = a.w + b.w;
}

void quaternion_subtract(Quaternion* dest, Quaternion a, Quaternion b) {
    dest->x = a.x - b.x;
    dest->y = a.y - b.y;
    dest->z = a.z - b.z;
    dest->w = a.w - b.w;
}

void quaternion_conjugate(Quaternion* dest, Quaternion q) {
    dest->x = -q.x;
    dest->y = -q.y;
    dest->z = -q.z;
    dest->w = q.w;
}

void quaternion_normalize(Quaternion* q) {
    gfc_vector4d_normalize(q);
}

void quaternion_multiply_q(Quaternion* dest, Quaternion a, Quaternion b) {
    dest->x = a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y;
    dest->y = a.w * b.y + a.y * b.w + a.z * b.x - a.x * b.z;
    dest->z = a.w * b.z + a.z * b.w + a.x * b.y - a.y * b.x;
    dest->w = a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z;
}

void quaternion_multiply_s(Quaternion* dest, Quaternion q, float s) {
    dest->x = q.x * s;
    dest->y = q.y * s;
    dest->z = q.z * s;
    dest->w = q.w * s;
}

void quaternion_multiply_v(Quaternion* dest, Quaternion q, GFC_Vector3D v) {
    dest->x = q.w * v.x + q.x * v.z - q.z * v.y;
    dest->y = q.w * v.y + q.y * v.x - q.x * v.z;
    dest->z = q.w * v.z + q.z * v.y - q.y * v.x;
    dest->w = -(q.x * v.x + q.y * v.y + q.z * v.z);
}

void quaternion_divide_s(Quaternion* dest, Quaternion q, float s) {
    dest->x = q.x / s;
    dest->y = q.y / s;
    dest->z = q.z / s;
    dest->w = q.w / s;
}

void quaternion_rotate_q(Quaternion* dest, Quaternion a, Quaternion b) {
    quaternion_multiply_q(dest, a, b);
    quaternion_conjugate(&a, a);
    quaternion_multiply_q(dest, *dest, a);
}

void quaternion_rotate_v(GFC_Vector3D* dest, Quaternion a, GFC_Vector3D v) {
    Quaternion tmp = {0};
    quaternion_multiply_v(&tmp, a, v);
    quaternion_conjugate(&a, a);
    quaternion_multiply_q(&tmp, tmp, a);
    dest->x = tmp.x;
    dest->y = tmp.y;
    dest->z = tmp.z;
}

void quaternion_from_axis_angle(Quaternion* dest, GFC_Vector3D axis, float a) {
    float half, s;
    gfc_vector3d_normalize(&axis);
    half = a * 0.5f;
    s = sinf(half);
    dest->x = axis.x * s;
    dest->y = axis.y * s;
    dest->z = axis.z * s;
    dest->x = cosf(half);
}

void quaternion_from_euler_angles(Quaternion* dest, float x, float y, float z) {
    Quaternion yaw, pitch, roll;
    quaternion_from_axis_angle(&pitch, gfc_vector3d(1, 0, 1), x);
    quaternion_from_axis_angle(&roll, gfc_vector3d(0, 1, 0), y);
    quaternion_from_axis_angle(&yaw, gfc_vector3d(0, 0, 1), z);
    *dest = yaw;
    quaternion_multiply_q(dest, *dest, pitch);
    quaternion_multiply_q(dest, *dest, roll);
}
