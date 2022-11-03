#include <SDL.h>

#include "simple_logger.h"
#include "gfc_input.h"
#include "gfc_vector.h"
#include "gfc_matrix.h"
#include "gfc_audio.h"

#include "gf3d_vgraphics.h"
#include "gf3d_pipeline.h"
#include "gf3d_swapchain.h"
#include "gf3d_model.h"
#include "gf3d_camera.h"
#include "gf3d_texture.h"
#include "gf3d_draw.h"
#include "gf3d_lights.h"

#include "gf2d_sprite.h"
#include "gf2d_font.h"
#include "gf2d_draw.h"
#include "gf2d_actor.h"
#include "gf2d_mouse.h"
#include "gf2d_windows.h"
#include "gf2d_windows_common.h"

#include "entity.h"
#include "pokemon.h"
#include "player.h"
#include "trainer.h"
#include "world.h"

extern int __DEBUG;
extern float TRAINER_X;
extern float TRAINER_Y;
extern float TRAINER_ROT_Z;

static int _done = 0;
static Window *_quit = NULL;

void onCancel(void *data)
{
    _quit = NULL;
}

void onExit(void *data)
{
    _done = 1;
    _quit = NULL;
}

void exitGame()
{
    _done = 1;
}

void exitCheck()
{
    if (_quit)
        return;
    _quit = window_yes_no("Exit?", onExit, onCancel, NULL);
}

int main(int argc, char *argv[])
{
    int a;

    World *w;
    Pokedex *pokedex;

    for (a = 1; a < argc; a++)
    {
        if (strcmp(argv[a], "--debug") == 0)
        {
            __DEBUG = 1;
        }
    }

    init_logger("gf3d.log", 0);
    gfc_input_init("config/input.cfg");
    slog("gf3d begin");
    gf3d_vgraphics_init("config/setup.cfg");
    gfc_audio_init(256, 16, 4, 1, 1, 1);
    gf2d_font_init("config/font.cfg");
    gf3d_draw_init();             // 3D
    gf2d_draw_manager_init(1000); // 2D
    gf2d_actor_init(1024);
    gf2d_windows_init(128, "config/windows.cfg");
    gf2d_mouse_load("assets/actors/mouse.actor");
    entity_system_init(1024);

    slog_sync();

    w = world_load("config/world.json");
    pokedex = load_pokedex_json("config/pokedex.json");


    pokemon_new(vector3d(800, 0, 0.0), vector3d(0.0, 0.0, -5 * M_PI / 10), pokedex->pokemon[0], 3);
    pokemon_new(vector3d(800 * 0.80901699437, 800 * 0.58778525229, 0.0), vector3d(0.0, 0.0, -3 * M_PI / 10), pokedex->pokemon[1], 1.2);
    pokemon_new(vector3d(800 * 0.30901699437, 800 * 0.95105651629, 0.0), vector3d(0.0, 0.0, -M_PI / 10), pokedex->pokemon[2], 90);
    pokemon_new(vector3d(800 * -0.30901699437, 800 * 0.95105651629, 0.0), vector3d(0.0, 0.0, M_PI / 10), pokedex->pokemon[3], 60);
    pokemon_new(vector3d(800 * -0.80901699437, 800 * 0.58778525229, 0.0), vector3d(0.0, 0.0, 3 * M_PI / 10), pokedex->pokemon[4], 3);
    pokemon_new(vector3d(800 * -1.0, 800 * 0.0, 0.0), vector3d(0.0, 0.0, 5 * M_PI / 10), pokedex->pokemon[5], 1.5);
    pokemon_new(vector3d(800 * -0.80901699437, 800 * -0.58778525229, 0.0), vector3d(0.0, 0.0, 7 * M_PI / 10), pokedex->pokemon[6], 90);
    pokemon_new(vector3d(800 * -0.30901699437, 800 * -0.95105651629, 0.0), vector3d(0.0, 0.0, 9 * M_PI / 10), pokedex->pokemon[7], 60);
    pokemon_new(vector3d(800 * 0.30901699437, 800 * -0.95105651629, 0.0), vector3d(0.0, 0.0, 11 * M_PI / 10), pokedex->pokemon[8], 90);
    pokemon_new(vector3d(800 * 0.80901699437, 800 * -0.58778525229, 0.0), vector3d(0.0, 0.0, 13 * M_PI / 10), pokedex->pokemon[9], 60);
    trainer_new(vector3d(0, 0, 0), vector3d(0, 0, M_PI), "calem", 120.0);

    slog_sync();
    gf3d_camera_set_scale(vector3d(1, 1, 1));
    player_new(vector3d(0, -1300, 600));

    // main game loop
    slog("gf3d main loop begin");
    while (!_done)
    {
        gfc_input_update();
        gf2d_mouse_update();
        gf2d_font_update();
        gf2d_windows_update_all();
        entity_think_all();
        entity_update_all();
        gf3d_camera_update_view();
        gf3d_camera_get_view_mat4(gf3d_vgraphics_get_view_matrix());
        // gf3d_camera_look_at(vector3d(100, 100, 100), vector3d(0, 0, 0), vector3d(0, 0, 1));

        // configure render command for graphics command pool
        // for each mesh, get a command and configure it from the pool
        gf3d_vgraphics_render_start();

        // 3D draws
        world_draw(w);
        entity_draw_all();
        // 2D draws
        gf2d_windows_draw_all();
        gf2d_mouse_draw();

        gf3d_vgraphics_render_end();

        if ((gfc_input_command_down("exit")) && (_quit == NULL))
        {
            _done = 1;
            // exitCheck();
        }
    }

    world_delete(w);

    vkDeviceWaitIdle(gf3d_vgraphics_get_default_logical_device());
    // cleanup
    slog("gf3d program end");
    slog_sync();
    return 0;
}

/*eol@eof*/
