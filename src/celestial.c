#include "simple_logger.h"
#include "gf3d_obj_load.h"

#include "celestial.h"

MeshPrimitive* generate_celestial_face(int resolution, GFC_Vector3D localUp) {
    ObjData* data;
    MeshPrimitive* prim;
    int y, x, i, faceIndex;
    GFC_Vector2D percent;
    GFC_Vector3D axisA, axisB, pointOnUnitCube, tmp;
    data = gf3d_obj_new();
    prim = gf3d_mesh_primitive_new();
    if (!data || !prim) {
        if (data) gf3d_obj_free(data);
        if (prim) free(prim);
        return NULL;
    }

    axisA = gfc_vector3d(localUp.z, localUp.x, localUp.y);
    gfc_vector3d_cross_product(&axisB, localUp, axisA);

    slog("local: %f %f %f", localUp.x, localUp.y, localUp.z);
    slog("axis a: %f %f %f", axisA.x, axisA.y, axisA.z);
    slog("axis b: %f %f %f", axisB.x, axisB.y, axisB.z);

    if (gfc_vector3d_magnitude(axisB) < 0.0001f)
        slog("BAD AXIS BASIS! axisB is zero vector");

    data->faceVertices = gfc_allocate_array(sizeof(Vertex), resolution * resolution);
    data->face_vert_count = resolution * resolution * 6;
    data->outFace = gfc_allocate_array(sizeof(Face), (resolution - 1) * (resolution - 1) * 2);
    data->face_count = (resolution - 1) * (resolution - 1) * 2;
    faceIndex = 0;

    for (y = 0; y < resolution; y++) {
        for (x = 0; x < resolution; x++) {
            i = x + y * resolution;
            percent = gfc_vector2d(x / (float) (resolution - 1), y / (float) (resolution - 1));

            gfc_vector3d_scale(tmp, axisA, (percent.x - .5f) * 2);
            gfc_vector3d_add(pointOnUnitCube, localUp, tmp);
            gfc_vector3d_scale(tmp, axisB, (percent.y - .5f) * 2);
            gfc_vector3d_add(pointOnUnitCube, pointOnUnitCube, tmp);
            gfc_vector3d_normalize(&pointOnUnitCube);

            gfc_vector3d_copy(data->faceVertices[i].vertex, pointOnUnitCube);
            data->faceVertices[i].texel.y = 0.5;
            gfc_vector3d_copy(data->faceVertices[i].normal, localUp);

            slog("pos %d: %f %f %f", i, pointOnUnitCube.x, pointOnUnitCube.y, pointOnUnitCube.z);

            if (x != resolution - 1 && y != resolution - 1)
            {
                data->outFace[faceIndex].verts[0] = i;
                data->outFace[faceIndex].verts[1] = i + resolution;
                data->outFace[faceIndex].verts[2] = i + resolution + 1;
                faceIndex++;

                data->outFace[faceIndex].verts[2] = i;
                data->outFace[faceIndex].verts[1] = i + 1;
                data->outFace[faceIndex].verts[0] = i + resolution + 1;
                faceIndex++;
            }
        }
    }
    
    prim->objData = data;
    gf3d_mesh_create_vertex_buffer_from_vertices(prim);
    gf3d_mesh_create_face_buffer_from_vertices(prim);
    return prim;
}

Mesh* generate_celestial_body(int resolution) {
    Mesh* mesh = gf3d_mesh_new();
    if (!mesh) return NULL;
    
    mesh->primitives = gfc_list_new_size(6);
    gfc_list_append(mesh->primitives, generate_celestial_face(resolution, gfc_vector3d(0, 0, 1)));
    gfc_list_append(mesh->primitives, generate_celestial_face(resolution, gfc_vector3d(0, 0, -1)));
    gfc_list_append(mesh->primitives, generate_celestial_face(resolution, gfc_vector3d(1, 0, 0)));
    gfc_list_append(mesh->primitives, generate_celestial_face(resolution, gfc_vector3d(-1, 0, 0)));
    gfc_list_append(mesh->primitives, generate_celestial_face(resolution, gfc_vector3d(0, 1, 0)));
    gfc_list_append(mesh->primitives, generate_celestial_face(resolution, gfc_vector3d(0, -1, 0)));

    return mesh;
}