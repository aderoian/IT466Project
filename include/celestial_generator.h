#ifndef __CELESTIAL_GENERATOR_H__
#define __CELESTIAL_GENERATOR_H__

#include "gfc_list.h"
#include "gfc_vector.h"

#include "gf3d_noise.h"

struct Mesh_S;

typedef struct ShapeSettings_S {
    float radius;
    int resolution;
    GFC_List* noiseLayers;
} ShapeSettings;

typedef enum NoiseSettingsType_E {
    Simple,
    Rigid
} NoiseSettingsType;

typedef struct NoiseSettings_S {
    NoiseSettingsType type;
    float strength;
    int numLayers;
    float baseRoughness;
    float roughness;
    float persistence;
    GFC_Vector3D center;
    float minValue;
    float weightMultiplier;
} NoiseSettings;

typedef struct NoiseLayer_S {
    Uint8 enabled;
    Uint8 firstIsMask;
    NoiseSettings settings;
} NoiseLayer;

struct Mesh_S* generate_celestial_body(const ShapeSettings* settings);

ShapeSettings* new_shape_settings(float radius, int resolution);
void free_shape_settings(ShapeSettings* settings);
NoiseLayer* new_noise_layer();
void create_simple_noise_settings(NoiseSettings* settings);

#endif // __CELESTIAL_H__