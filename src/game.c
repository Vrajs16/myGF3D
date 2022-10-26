#include <SDL.h>

#include "simple_logger.h"
#include "gfc_input.h"
#include "gfc_vector.h"
#include "gfc_matrix.h"

#include "gf3d_vgraphics.h"
#include "gf3d_pipeline.h"
#include "gf3d_swapchain.h"
#include "gf3d_model.h"
#include "gf3d_camera.h"
#include "gf3d_texture.h"
#include "gf3d_sprite.h"

#include "entity.h"
#include "pokemon.h"
#include "player.h"
#include "trainer.h"
#include "world.h"

extern int __DEBUG;

int main(int argc, char *argv[])
{
    int done = 0;
    int a;

    Sprite *mouse = NULL;
    int mousex, mousey;
    float mouseFrame = 0;
    World *w;

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
    slog_sync();

    entity_system_init(1024);

    mouse = gf3d_sprite_load("assets/pointer.png", 32, 32, 16);

    w = world_load("config/world.json");
    pokemon_new(vector3d(800, 0, 0.0), vector3d(0.0, 0.0, -5 * M_PI / 10), "growlithe", 3);
    pokemon_new(vector3d(800 * 0.80901699437, 800 * 0.58778525229, 0.0), vector3d(0.0, 0.0, -3 * M_PI / 10), "arcanine", 1.2);
    pokemon_new(vector3d(800 * 0.30901699437, 800 * 0.95105651629, 0.0), vector3d(0.0, 0.0, -M_PI / 10), "kirlia", 90);
    pokemon_new(vector3d(800 * -0.30901699437, 800 * 0.95105651629, 0.0), vector3d(0.0, 0.0, M_PI / 10), "gallade", 60);
    pokemon_new(vector3d(800 * -0.80901699437, 800 * 0.58778525229, 0.0), vector3d(0.0, 0.0, 3 * M_PI / 10), "krabby", 3);
    pokemon_new(vector3d(800 * -1.0, 800 * 0.0, 0.0), vector3d(0.0, 0.0, 5 * M_PI / 10), "kingler", 1.5);
    pokemon_new(vector3d(800 * -0.80901699437, 800 * -0.58778525229, 0.0), vector3d(0.0, 0.0, 7 * M_PI / 10), "skiddo", 90);
    pokemon_new(vector3d(800 * -0.30901699437, 800 * -0.95105651629, 0.0), vector3d(0.0, 0.0, 9 * M_PI / 10), "gogoat", 60);
    pokemon_new(vector3d(800 * 0.30901699437, 800 * -0.95105651629, 0.0), vector3d(0.0, 0.0, 11 * M_PI / 10), "zorua", 90);
    pokemon_new(vector3d(800 * 0.80901699437, 800 * -0.58778525229, 0.0), vector3d(0.0, 0.0, 13 * M_PI / 10), "zoroark", 60);
    trainer_new(vector3d(0, 0, 0), vector3d(0, 0, M_PI), "calem", 120.0);

    slog_sync();
    gf3d_camera_set_scale(vector3d(1, 1, 1));
    player_new(vector3d(0, -1300, 600));

    // main game loop
    slog("gf3d main loop begin");
    while (!done)
    {
        gfc_input_update();
        SDL_GetMouseState(&mousex, &mousey);

        mouseFrame += 0.01;
        if (mouseFrame >= 16)
            mouseFrame = 0;
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
        gf3d_sprite_draw(mouse, vector2d(mousex, mousey), vector2d(2, 2), (Uint32)mouseFrame);
        gf3d_vgraphics_render_end();

        if (gfc_input_command_down("exit"))
            done = 1; // exit condition
    }

    world_delete(w);

    vkDeviceWaitIdle(gf3d_vgraphics_get_default_logical_device());
    // cleanup
    slog("gf3d program end");
    slog_sync();
    return 0;
}

/*eol@eof*/
