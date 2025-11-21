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

extern int __DEBUG;

static int _done = 0;
static Uint32 frame_delay = 33;
static float fps = 0;

void parse_arguments(int argc,char *argv[]);
void game_frame_delay();

void exitGame()
{
    _done = 1;
}

int main(int argc,char *argv[])
{
    //local variables
    float now, then;
    Mesh* mesh;
    DefinitionData* def;
    Texture* texture;
    GFC_Matrix4 skyboxMat = {0};
    GFC_Vector3D light = {0, 0, 0};
    GFC_Vector3D cam = {0,-50,0};

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

    SDL_SetRelativeMouseMode(SDL_TRUE);
    gf2d_mouse_hide();

    gfc_matrix4_identity(skyboxMat);
    mesh = gf3d_mesh_load("models/sky/sky.obj");
    texture = gf3d_texture_load("models/sky/sky.png");

    def_init(1024);
    //def_load_directory("defs");
    ui_init();
    physics_init(2048);
    entity_init(2048);

    def_load("defs/test.def");
    def = def_load("defs/test.def");
    if (!def)
    {
        slog("Failed to load definition 'test'");
    } else {
        slog("Definition 'test' loaded:");
        sj_echo(def);
    }

    float size;
    int speed, kills;
    char *name;

    def_data_get_int(def, "speed", &speed),
    name = def_data_get_string(def, "name"),
    def_data_get_float(def, "size", &size),
    def_data_get_int(def_data_get_obj(def, "stats"), "kills", &kills);
    slog("speed: %d\nName: %s\nsize: %f\nkills: %d", speed, name, size, kills);

    cameraEntity = camera_entity_spawn(cam, NULL, 50, 10);

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
