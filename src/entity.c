#include "simple_logger.h"
#include "quaternion.h"

#include "entity.h"
#include "physics.h"

typedef struct {
    Entity* entities;
    Uint32 count;
} EntityManager;

extern int __DEBUG;
EntityManager entity_manager = {0};

void entity_init(Uint32 max_ents) {
    if (max_ents <= 0) {
        slog("Cannot init entity with zero entities");
        return;
    }

    entity_manager.entities = (Entity *)gfc_allocate_array(sizeof(Entity),max_ents);
    entity_manager.count = max_ents;

    if(__DEBUG)slog("entity manager initialized");
    atexit(entity_close);
}

void entity_close() {
    int i;
    for (i = 0; i < entity_manager.count;i++)
    {
        if (entity_manager.entities[i]._inuse > 0) {
            entity_free(&entity_manager.entities[i]);
        }
    }
    if (entity_manager.entities)
    {
        free(entity_manager.entities);
    }
    memset(&entity_manager,0,sizeof(EntityManager));
    if(__DEBUG)slog("entity manager closed");
}

Entity* entity_new() {
    int i;
    for (i = 0; i < entity_manager.count; i++) {
        if (entity_manager.entities[i]._inuse > 0)continue;
        entity_manager.entities[i]._inuse = 1;
        quaternion_identity(&entity_manager.entities[i].rotation);
        entity_manager.entities[i].color = GFC_COLOR_WHITE;
        entity_manager.entities[i].scale = gfc_vector3d(1, 1, 1);
        entity_manager.entities[i].draw = entity_draw;
        entity_manager.entities[i].physicsBody = NULL;
        return &entity_manager.entities[i];
    }

    return NULL;
}

void entity_free(Entity* ent) {
    if (!ent) return;

    if (ent->free) ent->free(ent);
    if (ent->mesh) gf3d_mesh_free(ent->mesh);
    if (ent->texture) gf3d_texture_free(ent->texture);
    if (ent->physicsBody) physics_body_free(ent->physicsBody);
    memset(ent,0,sizeof(Entity));
}

void entity_draw(Entity* ent, GFC_Vector3D lightPos, GFC_Color lightColor) {
    GFC_Matrix4 modelMat = {0};
    if (!ent) return;

    gfc_matrix4_from_vectors_q(
        modelMat,
        ent->position,
        ent->rotation,
        ent->scale
    );
    gf3d_mesh_draw(
        ent->mesh,
        modelMat,
        ent->color,
        ent->texture,
        lightPos,
        lightColor
    );
}

void entity_think(Entity* ent) {
    if (!ent || !ent->think) return;
    ent->think(ent);
}

void entity_update(Entity* ent) {
    if (!ent || !ent->update) return;
    ent->update(ent);
}

void entity_draw_all(GFC_Vector3D lightPos, GFC_Color lightColor) {
    int i;
    for (i = 0; i < entity_manager.count; i++) {
        if (entity_manager.entities[i]._inuse > 0) {
            if (entity_manager.entities[i].draw)
                entity_manager.entities[i].draw(&entity_manager.entities[i], lightPos, lightColor);
        }
    }
}

void entity_think_all() {
    int i;
    for (i = 0; i < entity_manager.count; i++) {
        if (entity_manager.entities[i]._inuse > 0) {
            entity_think(&entity_manager.entities[i]);
        }
    }
}

void entity_update_all() {
    int i;
    for (i = 0; i < entity_manager.count; i++) {
        if (entity_manager.entities[i]._inuse > 0) {
            entity_update(&entity_manager.entities[i]);
        }
    }
}
