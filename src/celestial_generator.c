#include "simple_logger.h"
#include "simple_json.h"
#include "gf3d_obj_load.h"

#include "celestial_generator.h"

#define clamp_01(n) ((n) > 1 ? 1 : ((n) < 0 ? 0 : (n)))

typedef struct ShapeSettingsManager_S {
    ShapeSettings* settings;
    Uint32 numSettings;
} ShapeSettingsManager;

MeshPrimitive* generate_celestial_face(const Noise* noise, const ShapeSettings* settings, GFC_Vector3D localUp) {
    ObjData* data;
    MeshPrimitive* prim;
    int y, x, i, faceIndex, resolution, xm, xp, ym, yp;
    float elevation;
    GFC_Vector2D percent;
    GFC_Vector3D axisA, axisB, pointOnUnitCube, tmp, pxm, pxp, pym, pyp, dx, dy, normal;
    if (!noise || !settings) return NULL;

    resolution = settings->resolution;
    data = gf3d_obj_new();
    prim = gf3d_mesh_primitive_new();
    if (!data || !prim) {
        if (data) gf3d_obj_free(data);
        if (prim) free(prim);
        return NULL;
    }

    axisA = gfc_vector3d(localUp.z, localUp.x, localUp.y);
    gfc_vector3d_cross_product(&axisB, localUp, axisA);

    data->face_count = resolution  * resolution * 2;
    data->outFace = gfc_allocate_array(sizeof(Face), data->face_count);
    data->face_vert_count = data->face_count * 3;
    data->faceVertices = gfc_allocate_array(sizeof(Vertex), data->face_vert_count);
    faceIndex = 0;

    if (!data->faceVertices || !data->outFace) {
        slog("ERROR: failed to allocate vertex (%p) and/or face buffer(%p).", data->faceVertices, data->outFace);
        gf3d_obj_free(data);
        free(prim);
        return NULL;
    }

    for (y = 0; y < resolution; y++) {
        for (x = 0; x < resolution; x++) {
            i = x + y * resolution;
            percent = gfc_vector2d(x / (float) (resolution - 1), y / (float) (resolution - 1));

            gfc_vector3d_scale(tmp, axisA, (percent.x - .5f) * 2);
            gfc_vector3d_add(pointOnUnitCube, localUp, tmp);
            gfc_vector3d_scale(tmp, axisB, (percent.y - .5f) * 2);
            gfc_vector3d_add(pointOnUnitCube, pointOnUnitCube, tmp);
            gfc_vector3d_normalize(&pointOnUnitCube);

            elevation = evaluate_noise(noise, settings, pointOnUnitCube);
            gfc_vector3d_copy(data->faceVertices[i].normal, pointOnUnitCube);
            gfc_vector3d_scale(pointOnUnitCube, pointOnUnitCube, elevation);

            gfc_vector3d_copy(data->faceVertices[i].vertex, pointOnUnitCube);
            gfc_vector2d_copy(data->faceVertices[i].texel, percent);
            if (x != resolution - 1 && y != resolution - 1)
            {
                data->outFace[faceIndex].verts[2] = i;
                data->outFace[faceIndex].verts[1] = i + resolution;
                data->outFace[faceIndex].verts[0] = i + resolution + 1;
                faceIndex++;

                data->outFace[faceIndex].verts[0] = i;
                data->outFace[faceIndex].verts[1] = i + 1;
                data->outFace[faceIndex].verts[2] = i + resolution + 1;
                faceIndex++;
            }
        }
    }

    // gfc_trigfc_angle_get_plane
    // try this?

    // Calculate normals
    // for (y = 1; y < resolution - 1; y++) {
    //     for (x = 1; x < resolution - 1; x++) {
    //         xm = (x - 1) + y * resolution;
    //         xp = (x + 1) + y * resolution;
    //         ym = x + (y - 1) * resolution;
    //         yp = x + (y + 1) * resolution;

    //         pxm = data->faceVertices[xm].vertex;
    //         pxp = data->faceVertices[xp].vertex;
    //         pym = data->faceVertices[ym].vertex;
    //         pyp = data->faceVertices[yp].vertex;

    //         gfc_vector3d_sub(dx, pxp, pxm);
    //         gfc_vector3d_sub(dy, pyp, pym);

    //         gfc_vector3d_cross_product(&data->faceVertices[i].normal, dx, dy);
    //         gfc_vector3d_normalize(&data->faceVertices[i].normal);
    //     }
    // }
    
    prim->objData = data;
    gf3d_mesh_create_vertex_buffer_from_vertices(prim);
    gf3d_mesh_create_face_buffer_from_vertices(prim);
    return prim;
}

Mesh* generate_celestial_body(const ShapeSettings* settings) {
    Mesh* mesh = gf3d_mesh_new();
    const Noise* noise = noise_new();
    if (!mesh) return NULL;
    
    mesh->primitives = gfc_list_new_size(6);
    gfc_list_append(mesh->primitives, generate_celestial_face(noise, settings, gfc_vector3d(0, 0, 1)));
    gfc_list_append(mesh->primitives, generate_celestial_face(noise, settings, gfc_vector3d(0, 0, -1)));
    gfc_list_append(mesh->primitives, generate_celestial_face(noise, settings, gfc_vector3d(1, 0, 0)));
    gfc_list_append(mesh->primitives, generate_celestial_face(noise, settings, gfc_vector3d(-1, 0, 0)));
    gfc_list_append(mesh->primitives, generate_celestial_face(noise, settings, gfc_vector3d(0, 1, 0)));
    gfc_list_append(mesh->primitives, generate_celestial_face(noise, settings, gfc_vector3d(0, -1, 0)));

    free(noise);
    return mesh;
}

float evaluate_noise_layer(const Noise* noise, const NoiseSettings* settings, GFC_Vector3D point) {
    float noiseValue, frequency, amplitude, weight, v;
    GFC_Vector3D noisePos;
    if (!noise || !settings) return 0;

    noiseValue = 0;
    frequency = settings->baseRoughness;
    amplitude = 1;
    weight = 1;

    for (int i = 0; i < settings->numLayers; i++) {
        if (settings->type == Simple) {
            gfc_vector3d_scale(noisePos, point, frequency);
            gfc_vector3d_add(noisePos, noisePos, settings->center);
            v = noise_evaluate(noise, noisePos);
            noiseValue += (v + 1) * .5f * amplitude;
        } else {
            gfc_vector3d_scale(noisePos, point, frequency);
            gfc_vector3d_add(noisePos, noisePos, settings->center);
            v = 1-fabsf(noise_evaluate(noise, noisePos));
            v *= v;
            v *= weight;
            weight = clamp_01(v * settings->weightMultiplier);

            noiseValue += v * amplitude;
        }
        frequency *= settings->roughness;
        amplitude *= settings->persistence;
    }

    noiseValue = noiseValue - settings->minValue;
    return noiseValue * settings->strength;
}

float evaluate_noise(const Noise* noise, const ShapeSettings* settings, GFC_Vector3D point) {
    float firstElevation = 0, elevation = 0;
    int layerCount, i;
    NoiseLayer* layer;
    if (!noise || !settings) return 0;

    layerCount = gfc_list_count(settings->noiseLayers);
    if (layerCount > 0) {
        layer = (NoiseLayer*) gfc_list_get_nth(settings->noiseLayers, 0);
        firstElevation = evaluate_noise_layer(noise, &layer->settings, point);
        if (layer->enabled) {
            elevation = firstElevation;
        }
    }

    for (i = 1; i < layerCount; i++) {
        layer = (NoiseLayer*) gfc_list_get_nth(settings->noiseLayers, i);
        if (layer->enabled) {
            elevation += evaluate_noise_layer(noise, &layer->settings, point) * (layer->firstIsMask ? firstElevation : 1);
        }
    }

    elevation = fmaxf(0, elevation);
    return settings->radius * (1 + elevation);
}

ShapeSettings* new_shape_settings(float radius, int resolution) {
    ShapeSettings* settings = gfc_allocate_array(sizeof(ShapeSettings), 1);
    NoiseLayer* layer;
    if (!settings) return NULL;
    settings->noiseLayers = gfc_list_new_size(1);
    if (!settings->noiseLayers) {
        free_shape_settings(settings);
        return NULL;
    }

    layer = new_noise_layer();
    if(!layer) {
        free_shape_settings(settings);
        return NULL;
    }

    gfc_list_append(settings->noiseLayers, new_noise_layer());
    settings->radius = radius;
    settings->resolution = resolution;
    return settings;
}

void free_shape_settings(ShapeSettings* settings) {
    int i;
    NoiseLayer* layer;
    if (!settings) return;

    if (settings->noiseLayers) {
        for (i = 0; i < gfc_list_count(settings->noiseLayers); i++) {
            layer = gfc_list_get_nth(settings->noiseLayers, i);
            if (layer) free(layer);
        }

        gfc_list_delete(settings->noiseLayers);
    }
}

NoiseLayer* new_noise_layer() {
    NoiseLayer* layer = gfc_allocate_array(sizeof(NoiseLayer), 1);
    if (!layer) return NULL;

    layer->enabled = 1;
    create_simple_noise_settings(&layer->settings);
    return layer;
}

void create_simple_noise_settings(NoiseSettings* settings) {
    if (!settings) return;

    settings->type = Simple;
    settings->strength = 1;
    settings->numLayers = 1;
    settings->baseRoughness = 1;
    settings->roughness = 2;
    settings->persistence = 0.5f;
}

ShapeSettings* shape_settings_from_json(SJson *json) {
    ShapeSettings* settings;
    NoiseLayer* noiseLayer;
    SJson * layers, *layer, *centerVal, *centerCoord;
    int layerCount = 0, i;
    if (!json) return NULL;
    settings = gfc_allocate_array(sizeof(ShapeSettings), 1);
    if (!settings) return NULL;

    sj_object_get_value_as_float(json, "radius", &settings->radius);
    sj_object_get_value_as_int(json, "resolution", &settings->resolution);
    layers = sj_object_get_value(json, "layers");
    layerCount = sj_array_get_count(layers);

    settings->noiseLayers = layerCount ? gfc_list_new_size(layerCount) : NULL;
    for (i = 0; i < layerCount; i++) {
        layer = sj_array_get_nth(layers, i);
        if (!layer) continue;

        noiseLayer = gfc_allocate_array(sizeof(NoiseLayer), 1);
        sj_object_get_value_as_uint8(layer, "enabled", &noiseLayer->enabled);
        sj_object_get_value_as_uint8(layer, "firstIsMask", &noiseLayer->firstIsMask);
        sj_object_get_value_as_int(layer, "noiseType", &noiseLayer->settings.type);
        sj_object_get_value_as_float(layer, "strenth", &noiseLayer->settings.strength);
        sj_object_get_value_as_int(layer, "numLayers", &noiseLayer->settings.numLayers);
        sj_object_get_value_as_float(layer, "baseRoughness", &noiseLayer->settings.baseRoughness);
        sj_object_get_value_as_float(layer, "roughness", &noiseLayer->settings.roughness);
        sj_object_get_value_as_float(layer, "persistence", &noiseLayer->settings.persistence);
        sj_object_get_value_as_float(layer, "minValue", &noiseLayer->settings.minValue);
        sj_object_get_value_as_float(layer, "weightMultiplier", &noiseLayer->settings.weightMultiplier);

        centerVal = sj_object_get_value(layer, "center");
        centerCoord = sj_array_get_nth(centerVal, 0);
        sj_get_float_value(centerCoord, &noiseLayer->settings.center.x);
        centerCoord = sj_array_get_nth(centerVal, 1);
        sj_get_float_value(centerCoord, &noiseLayer->settings.center.y);
        centerCoord = sj_array_get_nth(centerVal, 2);
        sj_get_float_value(centerCoord, &noiseLayer->settings.center.z);

        gfc_list_append(settings->noiseLayers, noiseLayer);
    }

    return settings;
}

SJson* shape_settings_to_json(const ShapeSettings* settings) {
    SJson *json, *value, *sLayers, *sLayer, *centerVal;
    int i;
    NoiseLayer *layer;
    if (!settings) return NULL;

    json = sj_object_new();

    value = sj_new_float(settings->radius);
    sj_object_insert(json, "radius", value);
    value = sj_new_int(settings->resolution);
    sj_object_insert(json, "resolution", value);

    sLayers = sj_array_new();
    for (i = 0; i < gfc_list_count(settings->noiseLayers); i++) {
        layer = gfc_list_nth(settings->noiseLayers, i);
        if (!layer) continue;

        sLayer = sj_object_new();
        value = sj_new_int(layer->enabled);
        sj_object_insert(sLayer, "enabled", value);
        value = sj_new_int(layer->firstIsMask);
        sj_object_insert(sLayer, "firstIsMask", value);
        value = sj_new_int(layer->settings.type);
        sj_object_insert(sLayer, "noiseType", value);
        value = sj_new_float(layer->settings.strength);
        sj_object_insert(sLayer, "strenth", value);
        value = sj_new_int(layer->settings.numLayers);
        sj_object_insert(sLayer, "numLayers", value);
        value = sj_new_float(layer->settings.baseRoughness);
        sj_object_insert(sLayer, "baseRoughness", value);
        value = sj_new_float(layer->settings.roughness);
        sj_object_insert(sLayer, "roughness", value);
        value = sj_new_float(layer->settings.persistence);
        sj_object_insert(sLayer, "persistence", value);
        value = sj_new_float(layer->settings.minValue);
        sj_object_insert(sLayer, "minValue", value);
        value = sj_new_float(layer->settings.weightMultiplier);
        sj_object_insert(sLayer, "weightMultiplier", value);

        centerVal = sj_array_new();
        value = sj_new_float(layer->settings.center.x);
        sj_array_append(centerVal, value);
        value = sj_new_float(layer->settings.center.y);
        sj_array_append(centerVal, value);
        value = sj_new_float(layer->settings.center.z);
        sj_array_append(centerVal, value);
        sj_object_insert(sLayer, "center", centerVal);

        sj_array_append(sLayers, sLayer);
    }

    sj_object_insert(json, "layers", sLayers);

    return json;
}