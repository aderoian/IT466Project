#ifndef __EDITOR_MENU_H__
#define __EDITOR_MENU_H__

#define NUM_NOISE_LAYER_VALUES 13

#include "ui.h"
#include "celestial_generator.h"
#include "gf2d_font.h"

typedef enum InputType_E {
    Int,
    Float,
    Boolean
} InputType;

typedef struct NoiseValue_S {
    InputType type;
    int min;
    int max;
    float step;

    void *data;
    char name[64];
    UIElement* element;
    void (*updateData) (InputType type, void *dest, void *src);
} NoiseValue;

typedef struct NoiseLayerContainer_S {
    NoiseLayer* layer;
    Uint8 expanded;
    float yPosition;
    NoiseValue values[NUM_NOISE_LAYER_VALUES];
} NoiseLayerContainer;

void editor_open(ShapeSettings* settings, struct Mesh_S **planet);

void editor_free_layer(NoiseLayerContainer* container);
NoiseLayerContainer* editor_create_layer(NoiseLayer* layer);

void editor_create_value(NoiseValue* val, InputType type, int min, int max, float step, void *data, char *name, void (*updateData) (InputType type, void *dest, void *src));

void editor_save(ShapeSettings *settings);


#endif // __EDITOR_MENU_H__