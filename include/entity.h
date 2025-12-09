#ifndef ENTITY_H
#define ENTITY_H

#include "gfc_vector.h"
#include "gfc_matrix.h"
#include "gfc_color.h"
#include "gf3d_mesh.h"
#include "gf3d_texture.h"

#include "quaternion.h"
#include "physics.h"

typedef struct Entity_s {
    Uint8 _inuse;
    GFC_TextLine name;
    Mesh* mesh;
    Texture *texture;
    GFC_Color color;
    GFC_Vector3D position;
    Quaternion rotation;
    GFC_Vector3D scale;
    GFC_Vector3D velocity;
    GFC_Box bounds;
    void (*think)(struct Entity_s *self);
    void (*update)(struct Entity_s *self);
    void (*free)(struct Entity_s *self);
    void (*draw)(struct Entity_s *self, GFC_Vector3D lightPos, GFC_Color lightColor);
    void* data;
    PhysicsBody* physicsBody;
} Entity;

void entity_close();
void entity_init(Uint32 max_ents);

Entity* entity_new();
void entity_free(Entity* ent);

void entity_draw(Entity* ent, GFC_Vector3D lightPos, GFC_Color lightColor);

void entity_draw_all(GFC_Vector3D lightPos, GFC_Color lightColor);
void entity_think_all();
void entity_update_all();

#endif
