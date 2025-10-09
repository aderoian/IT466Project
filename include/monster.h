#ifndef MONSTER_H
#define MONSTER_H

Entity* monster_spawn(GFC_Vector3D position, GFC_Color color);

void monster_think(Entity* ent);
void monster_update(Entity* ent);

#endif
