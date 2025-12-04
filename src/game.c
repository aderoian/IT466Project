#include <SDL.h>            

#include "simple_json.h"
#include "simple_logger.h"

#include "gfc_input.h"
#include "gfc_config_def.h"
#include "gfc_vector.h"
#include "gfc_matrix.h"
#include "gfc_audio.h"
#include "gfc_string.h"
#include "gfc_actions.h"

#include "gf2d_sprite.h"
#include "gf2d_font.h"
#include "gf2d_actor.h"
#include "gf2d_mouse.h"

#include "gf3d_vgraphics.h"
#include "gf3d_pipeline.h"
#include "gf3d_swapchain.h"

#include "gf3d_camera.h"
#include "gf3d_mesh.h"
#include "entity.h"
#include "physics.h"
#include "monster.h"
#include "player.h"
#include "camera_entity.h"
#include "quaternion.h"
#include "world.h"
#include "ui.h"
#include "overlay.h"
#include "world_map.h"
#include "main_menu.h"
#include "def.h"
#include "resource.h"
#include "civilization.h"
#include "mission_menu.h"
#include "celestial.h"
#include "celestial_entity.h"

extern int __DEBUG;

static int editor;

static int _done = 0;
static Uint32 frame_delay = 33;
static float fps = 0;

void parse_arguments(int argc,char *argv[]);
void game_frame_delay();

void exitGame()
{
    _done = 1;
}

void runGame() {
    //local variables
    float now, then;
    Mesh* mesh;
    Texture* texture;
    GFC_Matrix4 skyboxMat = {0};
    GFC_Vector3D light = {0, 0, 0};
    GFC_Vector3D cam = {0,-50,0};

    SDL_SetRelativeMouseMode(SDL_TRUE);
    gf2d_mouse_hide();

    gfc_matrix4_identity(skyboxMat);
    mesh = gf3d_mesh_load("models/sky/sky.obj");
    texture = gf3d_texture_load("models/sky/sky.png");

    def_init(1024);
    def_load_directory("defs"); // Load our default definitions

    resource_init();
    civilization_init();

    ui_init();
    physics_init(2048);
    entity_init(2048);

    mission_menu_init();

    cameraEntity = camera_entity_spawn(cam, NULL, 50, 10);
    //civilization_spawn(gfc_vector3d(0, 0, 0), civilization_get_by_name("Armen's Colony"));

    main_menu_load();
    ui_open_menu(main_menu_get());

    // main game loop
    now = SDL_GetTicks() / 1000.f;
    while(!_done)
    {
        gfc_input_update();
        gf2d_mouse_update();
        gf2d_font_update();

        // tick physics
        then = now;
        now = SDL_GetTicks() / 1000.f;
        physics_step(now - then);

        // world updates
        entity_think_all();
        entity_update_all();
        world_update();

        ui_update(now - then);

        //camera updaes
        gf3d_camera_update_view_q();
        gf3d_vgraphics_render_start();
                //3D draws
                gf3d_mesh_skybox_draw(mesh,skyboxMat,GFC_COLOR_WHITE,texture);
                entity_draw_all(light, GFC_COLOR_WHITE);

                gf2d_font_draw_line_tag("ALT+F4 to exit",FT_H1,GFC_COLOR_WHITE, gfc_vector2d(10,10));
                ui_draw();
                gf2d_mouse_draw();
        gf3d_vgraphics_render_end();
        if (gfc_input_command_down("exit"))_done = 1; // exit condition
        game_frame_delay();
    }    
    vkDeviceWaitIdle(gf3d_vgraphics_get_default_logical_device());    
    //cleanup
    slog("gf3d program end");
    exit(0);
    slog_sync();
}

void runPlanetEditor() {
    float now, then;
    GFC_Vector3D light = {0, 0, 0};
    GFC_Vector3D cam = {0,-50,0};
    Mesh* skyboxMesh, *planetMesh;
    Texture* skyboxTexture, *pTexture;
    GFC_Matrix4 skyboxMat = {0};
    Entity* planet;
    
    gfc_matrix4_identity(skyboxMat);
    skyboxMesh = gf3d_mesh_load("models/sky/sky.obj");
    skyboxTexture = gf3d_texture_load("models/sky/editor.png");

    ui_init();
    entity_init(64);

    planetMesh = generate_celestial_body(15);
    pTexture = gf3d_texture_load("models/primitives/flatwhite.png");
    planet = spawn_generated_celestial_entity(planetMesh, pTexture, gfc_vector3d(10, 10, 10));
    planet->position.y = 30;

    gf3d_camera_look_at(gfc_vector3d(0, 0, 0), &cam);

    // main game loop
    now = SDL_GetTicks() / 1000.f;
    while(!_done)
    {
        gfc_input_update();
        gf2d_mouse_update();
        gf2d_font_update();

        // world updates
        entity_think_all();
        entity_update_all();

        then = now;
        now = SDL_GetTicks() / 1000.f;
        ui_update(now - then);

        //camera updaes
        gf3d_camera_update_view();
        gf3d_vgraphics_render_start();
                //3D draws
                gf3d_mesh_skybox_draw(skyboxMesh,skyboxMat,GFC_COLOR_WHITE,skyboxTexture);
                entity_draw_all(light, GFC_COLOR_WHITE);

                gf2d_font_draw_line_tag("ALT+F4 to exit",FT_H1,GFC_COLOR_WHITE, gfc_vector2d(10,10));
                ui_draw();
                gf2d_mouse_draw();
        gf3d_vgraphics_render_end();
        if (gfc_input_command_down("exit"))_done = 1;
        game_frame_delay();
    }    
    vkDeviceWaitIdle(gf3d_vgraphics_get_default_logical_device());    
    //cleanup
    slog("gf3d program end");
    exit(0);
    slog_sync();
}

int main(int argc,char *argv[])
{
    //initializtion    
    parse_arguments(argc,argv);
    init_logger("gf3d.log",0);
    slog("gf3d begin");

    //gfc init
    gfc_input_init("config/input.cfg");
    gfc_config_def_init();
    gfc_action_init(1024);
    gfc_sound_init_config("config/audio.cfg");

    //gf3d init
    gf3d_vgraphics_init("config/setup.cfg");
    gf2d_font_init("config/font.cfg");
    gf2d_actor_init(1000);
    
    //game init
    srand(SDL_GetTicks());
    slog_sync();
    gf2d_mouse_load("actors/mouse.actor");

    if (!editor) {
        runGame();
    } else {
        runPlanetEditor();
    }

    return 0;
}

void parse_arguments(int argc,char *argv[])
{
    int a;

    for (a = 1; a < argc;a++)
    {
        if (strcmp(argv[a],"--debug") == 0)
        {
            __DEBUG = 1;
        } else if (strcmp(argv[a],"--planet-editor") == 0) {
            editor = 1;
        }
    }    
}

void game_frame_delay()
{
    Uint32 diff;
    static Uint32 now;
    static Uint32 then;
    then = now;
    slog_sync();// make sure logs get written when we have time to write it
    now = SDL_GetTicks();
    diff = (now - then);
    if (diff < frame_delay)
    {
        SDL_Delay(frame_delay - diff);
    }
    fps = 1000.0/MAX(SDL_GetTicks() - then,0.001);
//     slog("fps: %f",fps);
}
/*eol@eof*/
