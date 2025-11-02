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
    int i, j, k;
    float now, then;
    Mesh* mesh;
    Texture* texture;
    Entity* player;
    GFC_Vector3D playerPos = {0,0,0};
    GFC_Matrix4 skyboxMat = {0};
    GFC_Vector3D cam = {0,-50,0};
    GFC_Vector3D light = {100, 25, 30};

    //initializtion    
    parse_arguments(argc,argv);
    init_logger("gf3d.log",0);
    slog("gf3d begin");

    //gfc init
    gfc_input_init("config/input.cfg");
    gfc_config_def_init();
    gfc_action_init(1024);

    //gf3d init
    gf3d_vgraphics_init("config/setup.cfg");
    gf2d_font_init("config/font.cfg");
    gf2d_actor_init(1000);
    
    //game init
    srand(SDL_GetTicks());
    slog_sync();
    // bg = gf2d_sprite_load_image("images/bg_flat.png");
    gf2d_mouse_load("actors/mouse.actor");

    entity_init(2048);
    physics_init(2048);

    gfc_matrix4_identity(skyboxMat);
    mesh = gf3d_mesh_load("models/sky/sky.obj");
    texture = gf3d_texture_load("models/sky/sky.png");

    // main game loop
    //gf3d_camera_look_at(gfc_vector3d(0,0,0),&cam);

    int sep = 75;
    int size = 10;
    float start = -((size * sep) / 2);
    for (i = 0; i < size*sep; i+=sep) {
        for (j = 0; j < size*sep; j+=sep) {
            for (k = 0; k < size*sep; k+=sep) {
                if (start + i == 0 && start + j == 0 && start + k == 0) continue;
                monster_spawn(gfc_vector3d(start + i, start + j, start + k), gfc_color8(gfc_random_int(256), gfc_random_int(256), gfc_random_int(256), 255));
            }
        }
    }

    player = init_player(playerPos, GFC_COLOR_WHITE);
    camera_entity_spawn(cam, player, 50, 10);

    now = SDL_GetTicks() / 1000.f;
    while(!_done)
    {
        gfc_input_update();
        gf2d_mouse_update();
        gf2d_font_update();

        // tick physics
        then = now;
        now = SDL_GetTicks() / 1000.f;

        int start = SDL_GetTicks();
        physics_step(now - then);
        int end = SDL_GetTicks();
        //slog("total tick: %d", end-start);


        // world updates
        entity_think_all();
        entity_update_all();

        //camera updaes
        gf3d_camera_update_view_q();
        gf3d_vgraphics_render_start();
                //3D draws
                gf3d_mesh_skybox_draw(mesh,skyboxMat,GFC_COLOR_WHITE,texture);
                entity_draw_all(light, GFC_COLOR_WHITE);

                //2D draws
                // gf2d_sprite_draw_image(bg,gfc_vector2d(0,0));
                gf2d_font_draw_line_tag("ALT+F4 to exit",FT_H1,GFC_COLOR_WHITE, gfc_vector2d(10,10));
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
