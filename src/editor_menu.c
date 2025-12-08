#include "simple_logger.h"

#include "gfc_input.h"
#include "gf2d_mouse.h"
#include "gf3d_mesh.h"

#include "editor_menu.h"

#define INT_MIN -2147483648
#define INT_MAX 2147483647
#define FLOAT_MIN -3.4e+38f
#define FLOAT_MAX 3.4e+38f

#define LAYER_CLOSED_HEIGHT 30
#define LAYER_VALUE_HEIGHT 30
#define LAYER_EXPANDED_HEIGHT (LAYER_VALUE_HEIGHT * NUM_NOISE_LAYER_VALUES)
#define HEADER_VALUES_HEIGHT (LAYER_VALUE_HEIGHT * 2)
#define layer_height(expanded) (expanded ? (LAYER_EXPANDED_HEIGHT + LAYER_CLOSED_HEIGHT) : LAYER_CLOSED_HEIGHT)

#define translate_rect(dst, rect, pos) dst.x = (rect.x + pos.x); \
dst.y = (rect.y + pos.y)

#define clamp(val, min, max) ((val) < (min) ? (min) : ((val) > (max) ? (max) : (val)))

typedef struct EditorMenuManager_S {
    ShapeSettings* settings;
    UIElement* menu;
    UIElement* addButton;
    UIElement* saveButton;

    // Focus management for input fields
    NoiseValue* focused;
    char focusedVal[64];
    Uint8 mode; // 0 = keyboard, 1 = slidebar

    // Scroll management
    float offset;

    NoiseValue radius;
    NoiseValue resolution;
    int numLayers;
    NoiseLayerContainer** layerContainers;

    Mesh **planet;
    Uint8 dirty;
} EditorMenuManager;

void editor_populate_layers(ShapeSettings* settings);

static EditorMenuManager editorMenu = {0};

void editor_draw_layer_value(NoiseValue* value, GFC_Vector2D *pos) {
    char valueData[64];
    GFC_Vector2D valueDataPos;
    if (!value || !pos) return;

    // Draws background and label
    value->element->position = *pos;
    ui_draw_element(value->element);
    gf2d_font_draw_line_tag(value->name, FT_Small, GFC_COLOR_WHITE, *pos);

    // Draws current value
    // TODO: draw focused & updated value as its updated
    if (editorMenu.focused == value) {
        sprintf(valueData, "%s", editorMenu.focusedVal);
    } else {
        if (value->type == Float) {
        sprintf(valueData, "%f", *((float *)value->data));
    } else if (value->type == Boolean) {
        sprintf(valueData, "%d", *((char *)value->data));
    } else {
        sprintf(valueData, "%d", *((int *)value->data));
    }
    }
    gfc_vector2d_add(valueDataPos, (*pos), gfc_vector2d(175, 0));
    gf2d_font_draw_line_tag(valueData, FT_Small, GFC_COLOR_WHITE, valueDataPos);
    pos->y += LAYER_VALUE_HEIGHT;
}

void editor_draw_layer_container(NoiseLayerContainer* container, GFC_Vector2D *pos, int i) {
    int j;
    char label[16];
    if (!container || !pos) return;

    container->yPosition = pos->y;
    sprintf(label, "Layer %d", i);
    gf2d_font_draw_line_tag(label, FT_Small, GFC_COLOR_WHITE, *pos);
    pos->y += LAYER_VALUE_HEIGHT;

    if (container->expanded) {
        for (j = 0; j < NUM_NOISE_LAYER_VALUES; j++) {
            editor_draw_layer_value(&container->values[j], pos);
        }
    }
}

void editor_draw() {
    GFC_Vector2D pos;
    int i;
    if (!editorMenu.menu) return;
    pos = editorMenu.menu->position;
    gf2d_sprite_draw_image(editorMenu.menu->sprite, pos);

    pos.y -= editorMenu.offset + 9;

    editorMenu.saveButton->position.y = pos.y;
    gf2d_sprite_draw_image(editorMenu.saveButton->sprite, editorMenu.saveButton->position);
    pos.y += 50;

    editor_draw_layer_value(&editorMenu.radius, &pos);
    editor_draw_layer_value(&editorMenu.resolution, &pos);

    // Draw each layer container
    for (i = 0; i < editorMenu.numLayers; i++) {
        NoiseLayerContainer* container = editorMenu.layerContainers[i];
        if (!container) continue;
        editor_draw_layer_container(container, &pos, i);
    }

    editorMenu.addButton->position.y = pos.y;
    gf2d_sprite_draw_image(editorMenu.addButton->sprite, editorMenu.addButton->position);
}

void editor_click() {
    GFC_Vector2D mousePos = gf2d_mouse_get_position();
    float layersHeight = 0;
    int i, j;
    NoiseValue *value = NULL;
    NoiseLayer* layer;

    int dataInt;
    float dataFloat;
    Uint8 dataBool;

    value = &editorMenu.radius;
    if (!(mousePos.y >= value->element->position.y && mousePos.y < (value->element->position.y  + LAYER_VALUE_HEIGHT))) {
        value = NULL;
    }

    if (!value) {
        value = &editorMenu.resolution;
        if (!(mousePos.y >= value->element->position.y && mousePos.y < (value->element->position.y  + LAYER_VALUE_HEIGHT))) {
            value = NULL;
        }
    }

    for (i = 0; i < editorMenu.numLayers && !value; i++) {
        NoiseLayerContainer* container = editorMenu.layerContainers[i];
        if (!container) continue;

        if (mousePos.y >= container->yPosition && mousePos.y < (container->yPosition + LAYER_CLOSED_HEIGHT)) {
            container->expanded = !container->expanded;
        } else if (container->expanded) {
            for (j = 0; j < NUM_NOISE_LAYER_VALUES; j++) {
                value = &container->values[j];
                if (!(mousePos.y >= value->element->position.y && mousePos.y < (value->element->position.y  + LAYER_VALUE_HEIGHT))) {
                    value = NULL;
                } else {
                    break;
                }
            }
        }

        layersHeight += layer_height(container->expanded);
    }

    if (editorMenu.focused && editorMenu.mode == 0 && (!value || editorMenu.focused != value)) {
        if (editorMenu.focused->type == Int) {
            dataInt = atoi(editorMenu.focusedVal);
            editorMenu.focused->updateData(editorMenu.focused->type, editorMenu.focused->data, &dataInt);
        } else if (editorMenu.focused->type == Float) {
            dataFloat = atof(editorMenu.focusedVal);
            editorMenu.focused->updateData(editorMenu.focused->type, editorMenu.focused->data, &dataFloat);
        } else if (editorMenu.focused->type == Boolean) {
            dataBool = atoi(editorMenu.focusedVal) != 0;
            editorMenu.focused->updateData(editorMenu.focused->type, editorMenu.focused->data, &dataBool);
        }

        editorMenu.focused = NULL;
        editorMenu.focusedVal[0] = '\0';
        editorMenu.dirty = 1;
        return;
    } else if (!editorMenu.focused && value) {
        editorMenu.focused = value;
        editorMenu.mode = 0;
        return;
    }

    if (mousePos.y >= (editorMenu.addButton->position.y) && mousePos.y < (editorMenu.addButton->position.y + 23)) {
        
        layer = new_noise_layer();
        gfc_list_append(editorMenu.settings->noiseLayers, layer);
        editor_populate_layers(editorMenu.settings);
        editorMenu.dirty = 1;
    }

    if (mousePos.y > editorMenu.saveButton->position.y && mousePos.y < (editorMenu.saveButton->position.y + 35) &&
        mousePos.x > 1072 && mousePos.x < 1187) {
        editor_save(editorMenu.settings);
    }
}

void editor_update(float deltaTime) {
    int i, j;
    float deltaMove = 0, mouseMove = gf2d_mouse_get_movement().x, layersHeight = 0, dataMove = 0;
    GFC_Vector2D mousePos = gf2d_mouse_get_position();
    NoiseValue *value = NULL;

    int dataInt;
    Uint8 dataBool;
    float dataFloat;

    for (i = 0; i < editorMenu.numLayers; i++) {
        NoiseLayerContainer* container = editorMenu.layerContainers[i];
        if (!container) continue;
        layersHeight += layer_height(container->expanded);
    }

    if (layersHeight > 650);
    if (gfc_input_mouse_wheel_up()) {
        deltaMove -= 30;
    }
    if (gfc_input_mouse_wheel_down()) {
        deltaMove += 30;
    }

    editorMenu.offset = clamp((editorMenu.offset + deltaMove), 0, layersHeight);

    if (editorMenu.dirty) {
        gf3d_mesh_free(*editorMenu.planet);
        *editorMenu.planet = generate_celestial_body(editorMenu.settings);
        editorMenu.dirty = 0;
    }

    if(gf2d_mouse_button_held(0)) {
        if (!editorMenu.focused) {
            value = &editorMenu.radius;
            if (mousePos.x > 980 && mousePos.x <= (980 + 160) && mousePos.y >= value->element->position.y && mousePos.y < (value->element->position.y  + LAYER_VALUE_HEIGHT)) {
                editorMenu.focused = value;
                editorMenu.mode = 1;
            }

            value = &editorMenu.resolution;
            if (mousePos.x > 980 && mousePos.x <= (980 + 160) && mousePos.y >= value->element->position.y && mousePos.y < (value->element->position.y  + LAYER_VALUE_HEIGHT)) {
                editorMenu.focused = value;
                editorMenu.mode = 1;
            }

            for (i = 0; i < editorMenu.numLayers; i++) {
                NoiseLayerContainer* container = editorMenu.layerContainers[i];
                if (!container || !container->expanded) continue;
                for (j = 0; j < NUM_NOISE_LAYER_VALUES; j++) {
                    value = &container->values[j];
                    if (mousePos.x > 980 && mousePos.x <= (980 + 160) && mousePos.y >= value->element->position.y && mousePos.y < (value->element->position.y  + LAYER_VALUE_HEIGHT)) {
                        editorMenu.focused = value;
                        editorMenu.mode = 1;
                    }
                }
            }
        } else {
            if (mouseMove > 0) dataMove += editorMenu.focused->step;
            else if (mouseMove < 0) dataMove -= editorMenu.focused->step;

            if (dataMove) {
                if (editorMenu.focused->type == Int) {
                    dataInt = *((int *) editorMenu.focused->data) + (int) dataMove;
                    editorMenu.focused->updateData(editorMenu.focused->type, editorMenu.focused->data, &dataInt);
                } else if (editorMenu.focused->type == Float) {
                    dataFloat = (*((float *) editorMenu.focused->data) + dataMove);
                    editorMenu.focused->updateData(editorMenu.focused->type, editorMenu.focused->data, &dataFloat);
                } else {
                    dataBool = clamp((*((int *) editorMenu.focused->data) + (int) dataMove), 0, 1);
                    editorMenu.focused->updateData(editorMenu.focused->type, editorMenu.focused->data, &dataBool);
                }

                if (editorMenu.focused->type == Int) {
                    sprintf(editorMenu.focusedVal, "%d", *((int*) editorMenu.focused->data));
                } else if (editorMenu.focused->type == Float) {
                    sprintf(editorMenu.focusedVal, "%f", *((float*) editorMenu.focused->data));
                } else if (editorMenu.focused->type == Boolean) {
                    sprintf(editorMenu.focusedVal, "%d", *((char*) editorMenu.focused->data));
                }
                editorMenu.dirty = 1;
            }

        }
    }
    
    if (editorMenu.focused && editorMenu.mode == 0) {
        if (gfc_input_key_pressed("1")) {
            sprintf(editorMenu.focusedVal, "%s1", editorMenu.focusedVal);
        } else if (gfc_input_key_pressed("2")) {
            sprintf(editorMenu.focusedVal, "%s2", editorMenu.focusedVal);
        } else if (gfc_input_key_pressed("3")) {
            sprintf(editorMenu.focusedVal, "%s3", editorMenu.focusedVal);
        } else if (gfc_input_key_pressed("4")) {
            sprintf(editorMenu.focusedVal, "%s4", editorMenu.focusedVal);
        } else if (gfc_input_key_pressed("5")) {
            sprintf(editorMenu.focusedVal, "%s5", editorMenu.focusedVal);
        } else if (gfc_input_key_pressed("6")) {
            sprintf(editorMenu.focusedVal, "%s6", editorMenu.focusedVal);
        } else if (gfc_input_key_pressed("7")) {
            sprintf(editorMenu.focusedVal, "%s7", editorMenu.focusedVal);
        } else if (gfc_input_key_pressed("8")) {
            sprintf(editorMenu.focusedVal, "%s8", editorMenu.focusedVal);
        } else if (gfc_input_key_pressed("9")) {
            sprintf(editorMenu.focusedVal, "%s9", editorMenu.focusedVal);
        } else if (gfc_input_key_pressed("0")) {
            sprintf(editorMenu.focusedVal, "%s0", editorMenu.focusedVal);
        } else if (gfc_input_key_pressed(".")) {
            sprintf(editorMenu.focusedVal, "%s.", editorMenu.focusedVal);
        }
    }

    if (gf2d_mouse_button_released(0) && editorMenu.focused && editorMenu.mode == 1) {
        editorMenu.focused = NULL;
        editorMenu.focusedVal[0] = '\0';
    }
}

void editor_free_layers() {
    int i;
    for (i = 0; i < editorMenu.numLayers; i++) {
            editor_free_layer(editorMenu.layerContainers[i]);
    }

    editorMenu.numLayers = 0;
    free(editorMenu.layerContainers);
}

void editor_populate_layers(ShapeSettings* settings) {
    int i;
    if (!settings) return;

    if (editorMenu.layerContainers) {
        editor_free_layers();
    }

    editorMenu.numLayers = gfc_list_count(settings->noiseLayers);
    editorMenu.layerContainers = gfc_allocate_array(sizeof(NoiseLayerContainer*), editorMenu.numLayers);
    if (!editorMenu.layerContainers) {
        editorMenu.numLayers = 0;
        return;
    }

    for (i = 0; i < editorMenu.numLayers; i++) {
        NoiseLayer* layer = gfc_list_get_nth(settings->noiseLayers, i);
        editorMenu.layerContainers[i] = editor_create_layer(layer);
    }
}

void editor_open(ShapeSettings* settings, Mesh **planet) {
    if (!settings) return;

    editorMenu.settings = settings;
    editor_populate_layers(editorMenu.settings);

    editorMenu.menu = ui_element_create_simple("images/ui/editor/editor_background.png", gfc_vector2d(980, 0));
    editorMenu.menu->localBounding.x -= 141;
    editorMenu.menu->localBounding.x += 141;
    editorMenu.addButton = ui_element_create_simple("images/ui/editor/editor_layer_add.png", gfc_vector2d(984, 0));
    editorMenu.saveButton = ui_element_create_simple("images/ui/editor/editor_save.png", gfc_vector2d(1072, 18));
    editorMenu.menu->draw = editor_draw;
    editorMenu.menu->onClick = editor_click;
    editorMenu.menu->update = editor_update;

    editorMenu.planet = planet;

    editor_create_value(&editorMenu.radius, Float, INT_MAX, INT_MAX, 1, &settings->radius, "Radius", NULL);
    editor_create_value(&editorMenu.resolution, Int, INT_MAX, INT_MAX, 1, &settings->resolution, "Resolution", NULL);

    editorMenu.focused = NULL;
    editorMenu.mode = -1;
    editorMenu.focusedVal[0] = '\0';

    *planet = generate_celestial_body(settings);
    ui_open_menu(editorMenu.menu);
}

void editor_free_layer(NoiseLayerContainer* container) {
    int i;
    if (!container) return;
    for (i = 0; i < NUM_NOISE_LAYER_VALUES; i++) {
        if (container->values[i].element)
            ui_element_free(container->values[i].element);
    }
    free(container);
}

NoiseLayerContainer* editor_create_layer(NoiseLayer* layer) {
    NoiseLayerContainer* container;
    if (!layer) return NULL;
    container = gfc_allocate_array(sizeof(NoiseLayerContainer), 1);
    if (!container) return NULL;

    container->layer = layer;
    editor_create_value(&container->values[0], Boolean, 0, 1, 1, &layer->enabled, "Enabled", NULL);
    editor_create_value(&container->values[1], Boolean, 0, 1, 1, &layer->firstIsMask, "First Layer is Mask", NULL);
    editor_create_value(&container->values[2], Int, 1, 2, 1, &layer->settings.type, "Noise Type", NULL);
    editor_create_value(&container->values[3], Float, INT_MAX, INT_MAX, 1, &layer->settings.strength, "Strength", NULL);
    editor_create_value(&container->values[4], Int, INT_MAX, INT_MAX, 1, &layer->settings.numLayers, "Number of Layers", NULL);
    editor_create_value(&container->values[5], Float, INT_MAX, INT_MAX, 1, &layer->settings.baseRoughness, "Base Roughness", NULL);
    editor_create_value(&container->values[6], Float, INT_MAX, INT_MAX, 1, &layer->settings.roughness, "Roughness", NULL);
    editor_create_value(&container->values[7], Float, INT_MAX, INT_MAX, 1, &layer->settings.persistence, "Persistence", NULL);
    editor_create_value(&container->values[8], Float, INT_MAX, INT_MAX, 1, &layer->settings.center.x, "Center X", NULL);
    editor_create_value(&container->values[9], Float, INT_MAX, INT_MAX, 1, &layer->settings.center.y, "Center Y", NULL);
    editor_create_value(&container->values[10], Float, INT_MAX, INT_MAX, 1, &layer->settings.center.z, "Center Z", NULL);
    editor_create_value(&container->values[11], Float, INT_MAX, INT_MAX, 1, &layer->settings.minValue, "Min Value", NULL);
    editor_create_value(&container->values[12], Float, INT_MAX, INT_MAX, 1, &layer->settings.weightMultiplier, "Weight Multiplier", NULL);

    return container;
}

void editor_save(ShapeSettings *settings) {
    SJson *json;
    if (!settings) return;
    json = shape_settings_to_json(settings);
    if (!json) return;

    slog("saving body");
    sj_save(json, "shape_export.json");
}

void default_update_data(InputType type, void *dest, void *src) {
    switch (type) {
        case 0:
            *((int *) dest) = *((int *) src);
            break;
        case 1:
            *((float *) dest) = *((float *) src);
            break;
        case 2:
            *((char *) dest) = *((char *) src);
    }
}

void editor_create_value(NoiseValue* val, InputType type, int min, int max, float step, void *data, char *name, void (*updateData) (InputType type, void *dest, void *src)) {
    if (!val || !name) return;

    val->type = type;
    val->min = min;
    val->max = max;
    val->step = step;
    val->data = data;
    strcpy(val->name, name);
    val->updateData = val->updateData ? val->updateData : default_update_data;

    val->element = ui_element_create_simple("images/ui/editor/editor_input_background.png", gfc_vector2d(0, 0));
}