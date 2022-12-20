#include "gameloop.h"
#include "simple_logger.h"
#include "entity.h"
#include "gfc_audio.h"
#include "gfc_input.h"
#include "gf3d_vgraphics.h"
#include "gf2d_font.h"
#include "gf2d_draw.h"
#include "gf3d_draw.h"
#include "gf2d_actor.h"
#include "gf2d_windows.h"
#include "gf2d_mouse.h"

#include "world.h"
#include "pokemon.h"
#include "trainer.h"
#include "interactable.h"
#include "player.h"
#include "gf3d_camera.h"
#include "gf2d_windows_common.h"
#include "soundmanager.h"
#include "multiplayer.h"

#include "main_menu.h"
#include "content_editor.h"

extern int ROCK_COLLISION;
extern int TREE_COLLISION;
extern int SIGN_COLLISION;
extern int PC_COLLISION;
extern int STRENGTH_COLLISION;
extern int BATTLE;

extern int LOADING;

extern int MULTIPLAYER;

int DrawLoading = 0;

float HEALTH_RATE = 1;
int ANIMATION_PLAYING = 0;

extern Entity *OP_POKEMON;
extern float OP_HEALTH;
extern float NEW_OP_HEALTH;
extern float OP_HEALTH_MAX;
extern char OP_HEALTH_TEXT[5];

Entity *BATTLE_POKEMON;
float BATTLER_HEALTH;
float NEW_BATTLER_HEALTH;
float BATTLER_HEALTH_MAX;
char BATTLER_HEALTH_TEXT[5];
int BATTLER_POKEMON_DEAD;
short CAN_EVOLVE = -1;
int EVOLVE_ANIMATION = 0;

Sprite *BATTLE_SPRITE;

int CONTENT_EDITOR_GAME_DRAW = 0;

static Window *selectMoves = NULL;

State CurrentState = MAIN_MENU;

// Battle
void onRunSelected(void *data);
void onMoveSelected(void *data);

void gameloop_setup(void)
{
    slog("gf3d begin");

    init_logger("gf3d.log", 0);
    gfc_input_init("config/input.cfg");
    gf3d_vgraphics_init("config/setup.cfg");
    gfc_audio_init(256, 16, 4, 4, 1, 1);
    setSoundPack("config/sounds.cfg");

    gf2d_font_init("config/font.cfg");
    gf3d_draw_init();             // 3D
    gf2d_draw_manager_init(1000); // 2D
    gf2d_actor_init(1024);
    gf2d_windows_init(128, "config/windows.cfg");
    gf2d_mouse_load("assets/actors/mouse.actor");
    entity_system_init(1024);

    main_menu();
    playSound("lobby-music", -1, .6, 1, 1);
}

void gameloop_load(void)
{
    slog("LOADING GAME!");
    if (MULTIPLAYER)
        setup_connection();

    slog_sync();

    load_pokedex_json("config/pokedex.json");

    world_load_json("config/generated_world.json");

    srand(time(0));
    int r = 1;
    r = rand() % get_pokedex().total;
    BATTLE_POKEMON = pokemon_new(vector3d(0, -2000, -10000), vector3d(0, 0, M_PI), get_pokedex().pokemon[0], get_pokedex().pokemon[0].scale);
    BATTLER_HEALTH_MAX = (float)BATTLE_POKEMON->pokemon.health;
    BATTLER_HEALTH = BATTLER_HEALTH_MAX;
    NEW_BATTLER_HEALTH = BATTLER_HEALTH_MAX;
    TextLine pokemon_name;
    snprintf(pokemon_name, GFCLINELEN, "assets/content_editor/%s.png", BATTLE_POKEMON->pokemon.name);
    slog("POKEMON NAME: %s", pokemon_name);
    CAN_EVOLVE = BATTLE_POKEMON->pokemon.evolution;
    BATTLE_SPRITE = gf2d_sprite_load_image(pokemon_name);
    if (!BATTLE_SPRITE)
    {
        slog("Failed to load sprite: %s", pokemon_name);
    }

    sprintf(BATTLER_HEALTH_TEXT, "%d%%", (int)(BATTLER_HEALTH / BATTLER_HEALTH_MAX * 100));
    trainer_new(vector3d(0, 0, 0), vector3d(0, 0, 0), "calem", 200.0);
    slog_sync();
    gf3d_camera_set_scale(vector3d(1, 1, 1));
    player_new(vector3d(0, 1800, 1000));

    // main game loop
    slog("gf3d main loop begin");
    playSound("normal-music", -1, .3, 1, 1);
}

void gameloop_update(void)
{
    if (gfc_input_command_down("activate"))
    {
        if (CAN_EVOLVE != -1)
        {
            if (CAN_EVOLVE && !BATTLE)
            {
                EVOLVE_ANIMATION = 1;
                slog("POKEMON CAN EVOLVE!");
            }
        }
    }
    if (CONTENT_EDITOR_GAME_DRAW)
        content_editor_update();
    else
        gfc_input_update();
    gf2d_mouse_update();
    gf2d_font_update();
    gf2d_windows_update_all();
    entity_think_all();
    entity_update_all();
    entity_collide_check_all();
    gf3d_camera_update_view();
    gf3d_camera_get_view_mat4(gf3d_vgraphics_get_view_matrix());
    if (CurrentState == SINGLEPLAYER_GAME)
    {
        if (DrawLoading == 1)
        {
            CurrentState = LOADED_GAME;
            gameloop_load();
        }
        DrawLoading++;
    }
    if (CurrentState == MULTIPLAYER_GAME)
    {
        if (DrawLoading == 1)
        {
            CurrentState = LOADED_GAME;
            MULTIPLAYER = 1;
            gameloop_load();
        }
        DrawLoading++;
    }
    if (CurrentState == CONTENT_EDITOR_GAME)
    {
        if (DrawLoading == 1)
        {
            CurrentState = LOADED_GAME;
            content_editor_setup_renderer();
            CONTENT_EDITOR_GAME_DRAW = 1;
        }
        DrawLoading++;
    }
}

void gameloop_draw(void)
{
    if (CONTENT_EDITOR_GAME_DRAW)
        content_editor_draw();
    else
    {
        // configure render command for graphics command pool
        // for each mesh, get a command and configure it from the pool
        gf3d_vgraphics_render_start();

        // 3D draws
        world_draw();
        entity_draw_all();
        // 2D draws

        if (BATTLE)
        {
            if (OP_HEALTH > NEW_OP_HEALTH)
            {
                OP_HEALTH -= HEALTH_RATE;
                sprintf(OP_HEALTH_TEXT, "%d%%", (int)(OP_HEALTH / OP_HEALTH_MAX * 100));
            }
            else if (BATTLER_HEALTH > NEW_BATTLER_HEALTH)
            {
                BATTLER_HEALTH -= HEALTH_RATE;
                sprintf(BATTLER_HEALTH_TEXT, "%d%%", (int)(BATTLER_HEALTH / BATTLER_HEALTH_MAX * 100));
            }
            else
                ANIMATION_PLAYING = 0;

            if ((int)OP_HEALTH <= 0)
            {
                slog("You won");
                playSound("normal-music", -1, .3, 1, 1);
                BATTLE = 0;
                entity_free(entity_get(OP_POKEMON->name, OP_POKEMON->entityID));
            }
            else if ((int)BATTLER_HEALTH <= 0)
            {
                slog("You lost");
                playSound("normal-music", -1, .3, 1, 1);
                BATTLE = 0;
                entity_free(entity_get(OP_POKEMON->name, OP_POKEMON->entityID));
                BATTLER_POKEMON_DEAD = 1;
            }
            else
            {

                // Do some draw calls maybe to draw health and stuff

                // Draw the pokemon infobox opponent
                gf2d_draw_rect_filled(gfc_rect(20, 20, 300, 100), gfc_color8(255, 255, 255, 150));
                gf2d_draw_rect(gfc_rect(20, 20, 300, 100), gfc_color8(0, 0, 0, 255));
                // healthbar
                gf2d_draw_rect_filled(gfc_rect(30, 70, 200, 30), gfc_color8(100, 100, 100, 150));
                if ((int)(OP_HEALTH / OP_HEALTH_MAX * 100) > 0)
                {
                    if (OP_HEALTH / OP_HEALTH_MAX * 100 > 50)
                    {
                        gf2d_draw_rect_filled(gfc_rect(30, 70, OP_HEALTH / OP_HEALTH_MAX * 200, 30), gfc_color8(0, 255, 0, 200));
                    }
                    else if (OP_HEALTH / OP_HEALTH_MAX * 100 > 25)
                    {
                        gf2d_draw_rect_filled(gfc_rect(30, 70, OP_HEALTH / OP_HEALTH_MAX * 200, 30), gfc_color8(255, 255, 0, 200));
                    }
                    else
                    {
                        gf2d_draw_rect_filled(gfc_rect(30, 70, OP_HEALTH / OP_HEALTH_MAX * 200, 30), gfc_color8(255, 0, 0, 200));
                    }
                }
                gf2d_draw_rect(gfc_rect(30, 70, 200, 30), gfc_color8(255, 255, 255, 255));
                gf2d_font_draw_line_tag(OP_POKEMON->name, FT_Normal, gfc_color(0, 0, 0, 1), vector2d(30, 30));
                gf2d_font_draw_line_tag(OP_HEALTH_TEXT, FT_Small, gfc_color8(0, 0, 0, 255), vector2d(242.5, 67.5));

                // Draw the pokemon infobox player
                gf2d_draw_rect_filled(gfc_rect(20, gf3d_vgraphics_get_height() - 120, 300, 100), gfc_color8(255, 255, 255, 150));
                gf2d_draw_rect(gfc_rect(20, gf3d_vgraphics_get_height() - 120, 300, 100), gfc_color8(0, 0, 0, 255));
                // Health bar
                gf2d_draw_rect_filled(gfc_rect(30, gf3d_vgraphics_get_height() - 70, 200, 30), gfc_color8(100, 100, 100, 150));
                if ((int)(BATTLER_HEALTH / BATTLER_HEALTH_MAX * 100) > 0)
                {
                    if (BATTLER_HEALTH / BATTLER_HEALTH_MAX * 100 > 50)
                    {
                        gf2d_draw_rect_filled(gfc_rect(30, gf3d_vgraphics_get_height() - 70, BATTLER_HEALTH / BATTLER_HEALTH_MAX * 200, 30), gfc_color8(0, 255, 0, 200));
                    }
                    else if (BATTLER_HEALTH / BATTLER_HEALTH_MAX * 100 > 25)
                    {
                        gf2d_draw_rect_filled(gfc_rect(30, gf3d_vgraphics_get_height() - 70, BATTLER_HEALTH / BATTLER_HEALTH_MAX * 200, 30), gfc_color8(255, 255, 0, 200));
                    }
                    else
                    {
                        gf2d_draw_rect_filled(gfc_rect(30, gf3d_vgraphics_get_height() - 70, BATTLER_HEALTH / BATTLER_HEALTH_MAX * 200, 30), gfc_color8(255, 0, 0, 200));
                    }
                }
                gf2d_draw_rect(gfc_rect(30, gf3d_vgraphics_get_height() - 70, 200, 30), gfc_color8(255, 255, 255, 255));
                gf2d_font_draw_line_tag(BATTLE_POKEMON->name, FT_Normal, gfc_color8(0, 0, 0, 255), vector2d(30, gf3d_vgraphics_get_height() - 110));
                gf2d_font_draw_line_tag(BATTLER_HEALTH_TEXT, FT_Small, gfc_color8(0, 0, 0, 255), vector2d(242.5, gf3d_vgraphics_get_height() - 72.5));

                // Draw the attack buttons
                if (selectMoves == NULL && ANIMATION_PLAYING == 0)
                    selectMoves = battle_box(BATTLE_POKEMON->pokemon.moves, onMoveSelected, onRunSelected);
            }
        }
        else
        {
            // Need to draw battle pokemon health info percentage and name and avatar
            gf2d_draw_rect_filled(gfc_rect(30, gf3d_vgraphics_get_height() - 100, 300, 70), gfc_color8(255, 255, 255, 150));
            if (BATTLE_SPRITE)
            {
                gf2d_sprite_draw_image(BATTLE_SPRITE, vector2d(40, gf3d_vgraphics_get_height() - 90), NULL);
                // Health bar
                gf2d_draw_rect_filled(gfc_rect(100, gf3d_vgraphics_get_height() - 80, 200, 30), gfc_color8(100, 100, 100, 150));
                if ((int)(BATTLER_HEALTH / BATTLER_HEALTH_MAX * 100) > 0)
                {
                    if (BATTLER_HEALTH / BATTLER_HEALTH_MAX * 100 > 50)
                    {
                        gf2d_draw_rect_filled(gfc_rect(100, gf3d_vgraphics_get_height() - 80, BATTLER_HEALTH / BATTLER_HEALTH_MAX * 200, 30), gfc_color8(0, 255, 0, 200));
                    }
                    else if (BATTLER_HEALTH / BATTLER_HEALTH_MAX * 100 > 25)
                    {
                        gf2d_draw_rect_filled(gfc_rect(100, gf3d_vgraphics_get_height() - 80, BATTLER_HEALTH / BATTLER_HEALTH_MAX * 200, 30), gfc_color8(255, 255, 0, 200));
                    }
                    else
                    {
                        gf2d_draw_rect_filled(gfc_rect(100, gf3d_vgraphics_get_height() - 80, BATTLER_HEALTH / BATTLER_HEALTH_MAX * 200, 30), gfc_color8(255, 0, 0, 200));
                    }
                }
                gf2d_draw_rect(gfc_rect(100, gf3d_vgraphics_get_height() - 80, 200, 30), gfc_color8(255, 255, 255, 255));
            }
        }

        if (BATTLER_POKEMON_DEAD)
        {
            gf2d_draw_rect_filled(gfc_rect(10, 10, 330, 50), gfc_color8(128, 128, 128, 255));
            gf2d_font_draw_line_tag("Heal Pokemon To Battle!", FT_H2, gfc_color8(255, 255, 255, 255), vector2d(20, 20));
            gf2d_draw_rect(gfc_rect(10, 10, 330, 50), gfc_color8(255, 255, 255, 255));
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
        if (LOADING)
        {
            gf2d_draw_rect(gfc_rect(gf3d_vgraphics_get_width() / 2 - 200, gf3d_vgraphics_get_height() / 2 - 50, 400, 100), gfc_color8(0, 0, 0, 255));
            gf2d_draw_rect_filled(gfc_rect(gf3d_vgraphics_get_width() / 2 - 200, gf3d_vgraphics_get_height() / 2 - 50, 400, 100), gfc_color8(255, 255, 255, 80));
            gf2d_font_draw_line_tag("L O A D I N G...", FT_H1, gfc_color(1, 1, 1, 1), vector2d(gf3d_vgraphics_get_width() / 2 - 100, gf3d_vgraphics_get_height() / 2 - 12.5));
        }
        gf2d_mouse_draw();
        gf3d_vgraphics_render_end();
    }
}

void gameloop_close(void)
{
    world_delete();

    vkDeviceWaitIdle(gf3d_vgraphics_get_default_logical_device());
    // cleanup
    slog("gf3d program end");
    slog_sync();
}

void onMoveSelected(void *move)
{
    // print void callbackdata
    slog("%s used %s", BATTLE_POKEMON->name, ((Move *)move)->move);
    selectMoves = NULL;
    ANIMATION_PLAYING = 1;
    float multiplier = pokemon_move_multiplier(((Move *)move)->type, OP_POKEMON->pokemon.type);

    // do damage to opponent
    NEW_OP_HEALTH -= ((Move *)move)->power * multiplier;
    if (((Move *)move)->power > 0)
    {
        if (multiplier > 1)
        {
            slog("It's super effective!\n");
        }
        else if (multiplier == 1)
        {
            slog("It's effective!\n");
        }
        else if (multiplier < 1)
        {
            slog("It's not very effective!\n");
        }
    }
    if (NEW_OP_HEALTH < 0)
        return;

    // do damage to player
    Move moveSelected = OP_POKEMON->pokemon.moves[rand() % 4];
    slog("%s used %s", OP_POKEMON->name, moveSelected.move);
    multiplier = pokemon_move_multiplier(moveSelected.type, BATTLE_POKEMON->pokemon.type);
    NEW_BATTLER_HEALTH -= moveSelected.power * multiplier;
    if (moveSelected.power > 0)
    {
        if (multiplier > 1)
        {
            slog("It's super effective!\n");
        }
        else if (multiplier == 1)
        {
            slog("It's effective!\n");
        }
        else if (multiplier < 1)
        {
            slog("It's not very effective!\n");
        }
    }
    if (NEW_BATTLER_HEALTH < 0)
        return;
}

void onRunSelected(void *data)
{
    slog("%s ran away", BATTLE_POKEMON->name);
    selectMoves = NULL;
    playSound("normal-music", -1, .3, 1, 1);
    BATTLE = 0;
    entity_free(entity_get(OP_POKEMON->name, OP_POKEMON->entityID));
}
