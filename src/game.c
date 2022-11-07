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
#include "interactable.h"
#include "pokemon.h"
#include "player.h"
#include "trainer.h"
#include "world.h"

extern int __DEBUG;
extern int ROCK_COLLISION;
extern int TREE_COLLISION;
extern int SIGN_COLLISION;
extern int PC_COLLISION;
extern int STRENGTH_COLLISION;
extern int BATTLE;
extern Entity *OP_POKEMON;
int __BB = 0;

static int _done = 0;
static Window *_quit = NULL;
static Window *selectMoves = NULL;
Pokedex *pokedex = NULL;
void *callbackData = "hello";

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

void onMoveSelected(void *data)
{
    //print void callbackdata
    slog("callback data: %s", ((Move *) data)->move);
}

int main(int argc, char *argv[])
{
    int a;

    World *w;

    for (a = 1; a < argc; a++)
    {
        if (strcmp(argv[a], "--debug") == 0)
        {
            __DEBUG = 1;
        }
        if (strcmp(argv[a], "--bb") == 0 || strcmp(argv[a], "--BB") == 0)
        {
            __BB = 1;
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

    for (int i = 0; i < pokedex->total; i++)
    {
        pokemon_new(vector3d(950 * (i - pokedex->total / 2), 2000, 0.0), vector3d(0.0, 0.0, 0.0), pokedex->pokemon[i], pokedex->pokemon[i].scale);
    }
    srand(time(0));
    int r = 1;
    r = rand() % pokedex->total;
    Entity *battle_pok = pokemon_new(vector3d(0, -2000, 5000), vector3d(0, 0, M_PI), pokedex->pokemon[r], pokedex->pokemon[r].scale);
    trainer_new(vector3d(0, 0, 0), vector3d(0, 0, M_PI), "calem", 200.0);
    interactable_new(vector3d(-4000, -2000, 0), vector3d(0, 0, 0), "sign", 15);
    interactable_new(vector3d(-2000, -2000, 0), vector3d(0, 0, 0), "strength", 400);
    interactable_new(vector3d(0, -2000, 0), vector3d(0, 0, 0), "rock", 15);
    interactable_new(vector3d(2000, -2000, 0), vector3d(0, 0, 0), "pc", 250);
    interactable_new(vector3d(4000, -2000, 0), vector3d(0, 0, 0), "tree", 250);

    slog_sync();
    gf3d_camera_set_scale(vector3d(1, 1, 1));
    player_new(vector3d(0, -1800, 1000));

    // main game loop
    slog("gf3d main loop begin");
    char health[5];
    float health_value = 100;
    float health_decrease = 0.5;
    while (!_done)
    {
        gfc_input_update();
        gf2d_mouse_update();
        gf2d_font_update();
        gf2d_windows_update_all();
        entity_think_all();
        entity_update_all();
        entity_collide_check_all();
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

        if (BATTLE)
        {
            if (health_value > 0)
            {
                health_value -= health_decrease;
            }
            sprintf(health, "%d%%", (int)health_value);
            // Do some draw calls maybe to draw health and stuff

            // Draw the pokemon infobox opponent
            gf2d_draw_rect_filled(gfc_rect(200, 150, 300, 100), gfc_color8(255, 255, 255, 150));
            gf2d_draw_rect(gfc_rect(200, 150, 300, 100), gfc_color8(0, 0, 0, 255));
            // healthbar
            gf2d_draw_rect_filled(gfc_rect(210, 200, 200, 30), gfc_color8(100, 100, 100, 150));
            if (health_value != 0)
            {
                if (health_value > 50)
                {
                    gf2d_draw_rect_filled(gfc_rect(210, 200, health_value * 2, 30), gfc_color8(0, 255, 0, 200));
                }
                else if (health_value > 25)
                {
                    gf2d_draw_rect_filled(gfc_rect(210, 200, health_value * 2, 30), gfc_color8(255, 255, 0, 200));
                }
                else
                {
                    gf2d_draw_rect_filled(gfc_rect(210, 200, health_value * 2, 30), gfc_color8(255, 0, 0, 200));
                }
            }
            gf2d_draw_rect(gfc_rect(210, 200, 200, 30), gfc_color8(255, 255, 255, 255));
            gf2d_font_draw_line_tag(OP_POKEMON->name, FT_Normal, gfc_color(0, 0, 0, 1), vector2d(210, 160));
            gf2d_font_draw_line_tag(health, FT_Small, gfc_color8(0, 0, 0, 255), vector2d(425, 197.5));

            // Draw the pokemon infobox player
            gf2d_draw_rect_filled(gfc_rect(350, 425, 300, 100), gfc_color8(255, 255, 255, 150));
            gf2d_draw_rect(gfc_rect(350, 425, 300, 100), gfc_color8(0, 0, 0, 255));
            // Health bar
            gf2d_draw_rect_filled(gfc_rect(360, 475, 200, 30), gfc_color8(100, 100, 100, 150));
            if (health_value != 0)
            {
                if (health_value > 50)
                {
                    gf2d_draw_rect_filled(gfc_rect(360, 475, health_value * 2, 30), gfc_color8(0, 255, 0, 200));
                }
                else if (health_value > 25)
                {
                    gf2d_draw_rect_filled(gfc_rect(360, 475, health_value * 2, 30), gfc_color8(255, 255, 0, 200));
                }
                else
                {
                    gf2d_draw_rect_filled(gfc_rect(360, 475, health_value * 2, 30), gfc_color8(255, 0, 0, 200));
                }
            }
            gf2d_draw_rect(gfc_rect(360, 475, 200, 30), gfc_color8(255, 255, 255, 255));
            gf2d_font_draw_line_tag(battle_pok->name, FT_Normal, gfc_color8(0, 0, 0, 255), vector2d(360, 435));
            gf2d_font_draw_line_tag(health, FT_Small, gfc_color8(0, 0, 0, 255), vector2d(575, 472.5));

            // Draw the attack buttons
            if (selectMoves == NULL)
                selectMoves = battle_box(battle_pok->pokemon.moves, onMoveSelected);
        }

        if (SIGN_COLLISION)
        {
            gf2d_draw_rect_filled(gfc_rect(10, 10, 300, 50), gfc_color8(128, 128, 128, 255));
            gf2d_font_draw_line_tag("Vraj's Pokémon World!", FT_H2, gfc_color(1, 1, 1, 1), vector2d(20, 20));
            gf2d_draw_rect(gfc_rect(10, 10, 300, 50), gfc_color8(255, 255, 255, 255));
        }
        if (PC_COLLISION)
        {
            gf2d_draw_rect_filled(gfc_rect(10, 10, 430, 50), gfc_color8(128, 128, 128, 255));
            gf2d_font_draw_line_tag("Press 'E' to heal your Pokémon!", FT_H2, gfc_color(1, 1, 1, 1), vector2d(20, 20));
            gf2d_draw_rect(gfc_rect(10, 10, 430, 50), gfc_color8(255, 255, 255, 255));
        }
        if (STRENGTH_COLLISION)
        {
            gf2d_draw_rect_filled(gfc_rect(10, 10, 360, 50), gfc_color8(128, 128, 128, 255));
            gf2d_font_draw_line_tag("Press 'E' to use Strength!", FT_H2, gfc_color(1, 1, 1, 1), vector2d(20, 20));
            gf2d_draw_rect(gfc_rect(10, 10, 360, 50), gfc_color8(255, 255, 255, 255));
        }
        if (ROCK_COLLISION)
        {
            gf2d_draw_rect_filled(gfc_rect(10, 10, 390, 50), gfc_color8(128, 128, 128, 255));
            gf2d_font_draw_line_tag("Press 'E' to use Rock Smash!", FT_H2, gfc_color(1, 1, 1, 1), vector2d(20, 20));
            gf2d_draw_rect(gfc_rect(10, 10, 390, 50), gfc_color8(255, 255, 255, 255));
        }
        if (TREE_COLLISION)
        {
            gf2d_draw_rect_filled(gfc_rect(10, 10, 290, 50), gfc_color8(128, 128, 128, 255));
            gf2d_font_draw_line_tag("Press 'E' to use Cut!", FT_H2, gfc_color(1, 1, 1, 1), vector2d(20, 20));
            gf2d_draw_rect(gfc_rect(10, 10, 290, 50), gfc_color8(255, 255, 255, 255));
        }
        gf2d_windows_draw_all();
        gf2d_mouse_draw();

        // Box box1 = gfc_box(0, 0, 250, 500, 500, 500); //red
        // Box box2 = gfc_box(501, 0, 250, 500, 500, 500); //blue
        // gf3d_draw_cube_solid(box1, vector3d(0, 0, 0), vector3d(0, 0, 0), vector3d(.5, .5, .5), gfc_color8(255, 0, 0, 255));
        // gf3d_draw_cube_solid(box2, vector3d(0, 0, 0), vector3d(0, 0, 0), vector3d(.5, .5, .5), gfc_color8(0, 0, 255, 255));
        // if (gfc_box_overlap(box1, box2))
        // {
        //     slog("YES");
        // }
        gf3d_vgraphics_render_end();

        if ((gfc_input_command_down("exit")) && (_quit == NULL))
        {
            _done = 1;
            exitCheck();
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