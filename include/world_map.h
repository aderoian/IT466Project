#ifndef __WORLD_MAP_H__
#define __WORLD_MAP_H__

void world_map_load();
void world_map_set_player_location(int galaxyIndex, int solarSystemIndex);

struct UIElement_s* world_map_get();

#endif // __WORLD_MAP_H__
