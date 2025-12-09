#include <pthread.h>

#include "simple_logger.h"

#include "gfc_types.h"
#include "gfc_noise.h"
#include "gfc_vector.h"

#include "simple_json.h"

#include "player.h"
#include "celestial_entity.h"
#include "celestial_generator.h"
#include "world.h"

#define NUM_ASTEROIDS 400
#define MAX_ASTEROID_DISTANCE 75000

static Universe universe = {0};
static World world = {0};

void world_init() {
    world.solarSystem = NULL;
    world.asteroids = gfc_list_new_size(NUM_ASTEROIDS);

    for (int i = 0 ; i < NUM_ASTEROIDS; i++) {
        gfc_list_append(world.asteroids, NULL);
    }
}

void world_despawn_solarSystem();
Entity* world_create_asteroid(GFC_Vector3D center, SolarSystem* ss);

void world_close() {
    int i;
    Entity* body;
    world_despawn_solarSystem();

    for (i = 0; i < gfc_list_count(world.asteroids); i++) {
        body = (Entity*) gfc_list_get_nth(world.asteroids, i);
        if (body) entity_free(body);
    }
    gfc_list_delete(world.asteroids);
}

void world_update() {
    int i;
    float mag;
    Entity* body;
    if (!player || !world.solarSystem) return;

    // Remove stale asteroids (far from player)
    // for (i = 0; i < NUM_ASTEROIDS && world.numAsteroids > 0; i++) {
    //     body = (Entity*) gfc_list_get_nth(world.asteroids, i);
    //     if (!body) continue;

    //     if (gfc_vector3d_magnitude_between_squared(body->position, player->position) < MAX_ASTEROID_DISTANCE * MAX_ASTEROID_DISTANCE) {
    //         if (body->_inuse == 0) gfc_list_set_nth(world.asteroids, i, NULL);
    //         continue;
    //     }

    //     if (body) entity_free(body);
    //     gfc_list_set_nth(world.asteroids, i, NULL);
    //     world.numAsteroids--;
    // }

    // // Create new asteroids to fill to cap
    // for (i = 0; i < NUM_ASTEROIDS && world.numAsteroids < NUM_ASTEROIDS; i++) {
    //     body = (Entity*) gfc_list_get_nth(world.asteroids, i);
    //     if (body) continue;

    //     // Create asteroid
    //     body = world_create_asteroid(player->position, world.solarSystem);
    //     gfc_list_set_nth(world.asteroids, i, body);
    //     world.numAsteroids++;
    // }
}

void world_load_universe(Universe* universe) {
    // TODO
}
void world_save_universe(Universe* universe) {
    // TODO
}

World* world_get() {
    return &world;
}

Universe* world_get_universe() {
    return &universe;
}

void world_spawn_solarSystem() {
    int i;
    SolarSystem* ss = world_get_target_solarSystem();
    CelestialBody* body;
    Entity* ent;
    if (!ss) return;

    for (i = 0; i < ss->numBodies; i++) {
        body = ss->celestialBodies[i];
        if (!body) continue;
        ent = spawn_celestial_entity(body);
        if (!ent) {
            slog("Failed to spawn celestial entity.");
            continue;
        }
        body->entity = ent;
    }
}

void world_despawn_solarSystem() {
    int i;
    SolarSystem* ss = world_get_target_solarSystem();
    CelestialBody* body;
    if (!ss) return;

    for (i = 0; i < ss->numBodies; i++) {
        body = ss->celestialBodies[i];
        if (!body || !body->entity) continue;
        entity_free(body->entity);
    }

    world.solarSystem = NULL;

    for (i = 0; i < gfc_list_count(world.asteroids); i++) {
        Entity* body = (Entity*) gfc_list_get_nth(world.asteroids, i);
        if (body) {
            entity_free(body);
            gfc_list_set_nth(world.asteroids, i, NULL);
        }
    }
    world.numAsteroids = 0;
}

SolarSystem* world_get_target_solarSystem() {
    return world.solarSystem;
}

void world_set_target_solarSystem(SolarSystem* solarSystem) {
    if (!solarSystem) return;
    world_despawn_solarSystem();
    world.solarSystem = solarSystem;
    world_spawn_solarSystem();
}

typedef struct SolarSystemTask_s {
    int width, height;
    Galaxy** galaxies;
    int numGalaxies;
} SolarSystemTask;

void world_generate_galaxies(Universe *universe, int width, int height);
void* world_generate_solarSystems(void* input);
void world_generate_solarSystem(SolarSystem* solarSystem);
void world_generate_planeOfPoints(GFC_List *points, int width, int heigh, int R);

void world_generate_universe(Universe *out, int width, int height) {
    int i, j, k = 0, nThreads, numOps;
    Galaxy *gal;
    SolarSystem *ss;
    pthread_t* threads;
    SolarSystemTask* task;
    if (!out) return;

    world_generate_galaxies(out, width, height);

    nThreads = fmin(fmax(1, out->numGalaxies / 10), 10);
    threads = gfc_allocate_array(sizeof(pthread_t), nThreads);
    numOps = (out->numGalaxies / 10) + 1;

    for (i = 0; i < nThreads; i++) {
        task = gfc_allocate_array(sizeof(SolarSystemTask), 1);
        task->width = width;
        task->height = height;
        task->galaxies = gfc_allocate_array(sizeof(Galaxy*), numOps);
        for(j = 0; j < numOps && k < out->numGalaxies; j++) {
            task->galaxies[j] = out->galaxies[k];
            task->numGalaxies++;
            k++;
        }
        pthread_create(&threads[i], NULL, world_generate_solarSystems, (void *) task);
    }

    for (i = 0; i < nThreads; i++) {
        pthread_join(threads[i], NULL);
    }

    free(threads);

    for (i = 0; i < out->numGalaxies; i++) {
        gal = out->galaxies[i];
        if (gal) {
            for (j = 0; j < gal->numSolarSystems; j++) {
                ss = gal->solarSystems[j];
                if (ss) {
                    world_generate_solarSystem(ss);
                }
            }
        }
    }
}

void world_generate_galaxies(Universe *universe, int width, int height) {
    GFC_List *points;
    int count, x;
    Galaxy* gal;
    GFC_Vector2D *point;
    if (!universe) return;

    points = gfc_list_new();
    world_generate_planeOfPoints(points, width, height, 100);

    count = gfc_list_count(points);
    universe->galaxies = gfc_allocate_array(sizeof(Galaxy*), count);
    for (x = 0; x < count; x++) {
        point = gfc_list_get_nth(points, x);
        gal = gfc_allocate_array(sizeof(Galaxy), 1);
        gfc_vector2d_copy((*gal).pos, (*point));
        universe->galaxies[x] = gal;
        free(point);
    }
    universe->numGalaxies = count;

    gfc_list_delete(points);
}

void* world_generate_solarSystems(void* task) {
    Galaxy** galaxies;
    GFC_List *points;
    int width, height, numGalaxies, i, count, x;
    SolarSystem* ss;
    GFC_Vector2D *point;

    galaxies = ((SolarSystemTask*) task)->galaxies;
    width = ((SolarSystemTask*) task)->width;
    height = ((SolarSystemTask*) task)->height;
    numGalaxies = ((SolarSystemTask*) task)->numGalaxies;

    for (i = 0; i < numGalaxies; i++) {
        points = gfc_list_new();
        world_generate_planeOfPoints(points, width, height, 200);

        count = gfc_list_count(points);
        galaxies[i]->solarSystems = gfc_allocate_array(sizeof(SolarSystem*), count);
        for (x = 0; x < count; x++) {
            point = gfc_list_get_nth(points, x);
            ss = gfc_allocate_array(sizeof(SolarSystem), 1);
            gfc_vector2d_copy((*ss).pos, (*point));
            galaxies[i]->solarSystems[x] = ss;
            free(point);
        }
        galaxies[i]->numSolarSystems = count;

        gfc_list_delete(points);
    }

    free(galaxies);
    free(task);
    return NULL;
}

void world_get_planet_texture(char* out, int i) {
    switch (i) {
        case 0:
            strcpy(out, "images/celestial/earth.jpg");
            break;
        case 1:
            strcpy(out, "images/celestial/ceres.jpg");
            break;
        case 2:
            strcpy(out, "images/celestial/makemake.jpg");
            break;
        case 3:
            strcpy(out, "images/celestial/eris.jpg");
            break;
        case 4:
            strcpy(out, "images/celestial/haumea.jpg");
            break;
        case 5:
            strcpy(out, "images/celestial/jupiter.jpg");
            break;
        case 6:
            strcpy(out, "images/celestial/mars.jpg");
            break;
        case 7:
            strcpy(out, "images/celestial/mercury.jpg");
            break;
        case 8:
            strcpy(out, "images/celestial/neptune.jpg");
            break;
        case 9:
            strcpy(out, "images/celestial/saturn.jpg");
            break;
        case 10:
            strcpy(out, "images/celestial/uranus.jpg");
            break;
        case 11:
            strcpy(out, "images/celestial/venus_atmosphere.jpg");
            break;
        case 12:
            strcpy(out, "images/celestial/venus_surface.jpg");
            break;
    }
}

void world_generate_solarSystem(SolarSystem* ss) {
    int i, j, numPlanets, numMoons, totalNumMoons = 0;
    float totalRadiusDistance = 45, angle, dx, dy, totalMoonRadiusDistance = 0;
    CelestialBody *body, *moon;
    SJson *shapeData;
    GFC_List* bodies;
    if (!ss) return;

    numPlanets = gfc_random_int(10) + 5 + 1;
    bodies = gfc_list_new();

    for (i = 0; i < numPlanets; i++) {
        body = gfc_allocate_array(sizeof(CelestialBody), 1);
        gfc_list_append(bodies, body);
        body->type = PLANET;
        strcpy(body->name, "celestial");
        world_get_planet_texture(body->texture, gfc_random_int(13));
        //strcpy(body->texture, "models/primitives/flatwhite.png");
        //body->mass = gfc_random() * 10;
        body->mass = 10.f;
        shapeData = sj_load("defs/shapes/default.json");
        body->settings = shape_settings_from_json(shapeData);
        if (!body->settings) slog ("Failed to load shape settings.");
        free(shapeData);

        //totalRadiusDistance += (gfc_random() * 15) + 20;
        totalRadiusDistance += 30;
        angle = gfc_random() * GFC_2PI;
        dx = cosf(angle) * totalRadiusDistance;
        dy = sinf(angle) * totalRadiusDistance;
        body->pos.x = dx;
        body->pos.y = dy;

        numMoons = gfc_random_int(2);
        for (j = 0; j < numMoons; j++) {
            moon = gfc_allocate_array(sizeof(CelestialBody), 1);
            gfc_list_append(bodies, moon);
            moon->type = MOON;
            strcpy(moon->name, "celestial");
            strcpy(moon->texture, "images/celestial/moon.jpg");
            //strcpy(body->texture, "models/primitives/flatwhite.png");
            moon->mass = gfc_random() * 4 + 1;
            //moon->radius = gfc_random() * 15;
            shapeData = sj_load("defs/shapes/default.json");
            moon->settings = shape_settings_from_json(shapeData);
            free(shapeData);

            //totalMoonRadiusDistance += (gfc_random() * 4) + 1;
            totalMoonRadiusDistance += 15.f;
            angle = gfc_random() * GFC_2PI;
            dx = cosf(angle) * totalMoonRadiusDistance;
            dy = sinf(angle) * totalMoonRadiusDistance;
            moon->pos.x = body->pos.x + dx;
            moon->pos.y = body->pos.y + dy;
        }
        totalNumMoons += numMoons;
    }

    ss->celestialBodies = gfc_allocate_array(sizeof(CelestialBody*), numPlanets + totalNumMoons);
    for (i = 0; i < gfc_list_count(bodies); i++) {
        ss->celestialBodies[i] = (CelestialBody*) gfc_list_get_nth(bodies, i);
    }
    ss->numBodies = numPlanets + totalNumMoons;

    gfc_list_delete(bodies);
}

void world_generate_planeOfPoints(GFC_List *points, int width, int height, int R) {
    int i, x, y, p = 0;
    float nx, ny;
    GFC_Vector2D *point, *pointCmp;

    float bluenoise[width][height];

    for (x = 0; x < width; x++) {
        for (y = 0; y < height; y++) {
            nx = (float)x/width - 0.5;
            ny = (float)y/height - 0.5;
            bluenoise[x][y] = (gfc_perlin(gfc_vector2d(50 * nx, 50 * ny)) * 0.5) + 0.5;
        }
    }

    int t = 0,a = 0;
    for (x = 0; x < width; x++) {
        for (y = 0; y < height; y++) {
            if (bluenoise[x][y] > 0.6) {
                t++;
                point = gfc_allocate_array(sizeof(GFC_Vector2D), 1);
                point->x = x;
                point->y = y;

                if (gfc_list_count(points) == 0) {
                    gfc_list_append(points, point);
                    continue;
                }

                p = 1;
                for (i = 0; i < gfc_list_count(points); i++) {
                    pointCmp = gfc_list_get_nth(points, i);
                    if (gfc_vector2d_magnitude_between_squared(*point, *pointCmp) < R * R) {
                        p = 0;
                        break;
                    }
                }

                if (p) {
                    a++;
                    gfc_list_append(points, point);

                }
            }
        }
    }
}

Entity* world_create_asteroid(GFC_Vector3D center, SolarSystem* ss) {
    int distance;
    float theta, z, r, size;
    GFC_Vector3D pos;
    if (!ss) return NULL;

    distance = gfc_random_int(MAX_ASTEROID_DISTANCE - 500) + 500;
    theta = gfc_random() * GFC_2PI;
    z = gfc_random() * 2.0f - 1.0f;
    r = sqrtf(1.0f - z * z);

    pos = gfc_vector3d(r * cosf(theta), z, r * sinf(theta));

    gfc_vector3d_scale(pos, pos, distance);
    gfc_vector3d_add(pos, pos, center);

        // TODO: Check collision with celestial bodies
    size = 2 * ((gfc_random() * 10.f) + 10.f);
    return spawn_asteroid(pos, size);
}
