#include "simple_logger.h"
#include "simple_json.h"

#include "gfc_types.h"
#include "gfc_config.h"

#include "gf3d_lights.h"

#include "entity.h"
#include "pokemon.h"
#include "interactable.h"
#include "world.h"
/*
typedef struct
{

    Model *worldModel;
    List *spawnList;        //entities to spawn
    List *entityList;       //entities that exist in the world
}World;
*/

static World *MAIN_WORLD = NULL;

int WORLD_BOUND_X1;
int WORLD_BOUND_Y1;

int WORLD_BOUND_X2;
int WORLD_BOUND_Y2;

void world_draw()
{
    if (!MAIN_WORLD)
        return;
    if (!MAIN_WORLD->worldModel)
    {
        slog("world has no model");
        return; // no model to draw, do nothing
    }
    if (!MAIN_WORLD->sky)
        slog("world has no sky");
    else
        gf3d_model_draw_sky(MAIN_WORLD->sky, MAIN_WORLD->skyMat, gfc_color(1, 1, 1, 1));
    for (int i = 0; i < MAIN_WORLD->entityCount; i++)
    {
        gf3d_model_draw(MAIN_WORLD[i].worldModel, MAIN_WORLD[i].modelMat, gfc_color_to_vector4f(MAIN_WORLD[i].color), vector4d(0, 0, 0, 0));
    }
}

void world_delete()
{
    if (!MAIN_WORLD)
        return;
    gf3d_model_free(MAIN_WORLD->worldModel);
    free(MAIN_WORLD);
}

void world_run_updates(World *world);

void world_add_entity(World *world, Entity *entity);

void world_load_json(char *filename)
{
    // load the world json file
    SJson *json_file;
    int scale;
    int spacing;
    int grid_width;
    int grid_height;
    json_file = sj_load(filename);
    if (!json_file)
    {
        slog("failed to load world file %s", filename);
        return;
    }
    sj_object_get_value_as_int(json_file, "scale", &scale);
    sj_object_get_value_as_int(json_file, "spacing", &spacing);
    sj_object_get_value_as_int(json_file, "grid_width", &grid_width);
    sj_object_get_value_as_int(json_file, "grid_height", &grid_height);

    SJson *world = sj_object_get_value(json_file, "world");
    if (!world)
    {
        slog("failed to find world in %s world config", filename);
        sj_free(json_file);
        return;
    }
    int tilecount = sj_array_get_count(world);

    // load battle box
    SJson *json_file2;
    int scale2;
    int spacing2;
    int grid_width2;
    int grid_height2;
    json_file2 = sj_load("config/battle_box.json");

    if (!json_file2)
    {
        slog("failed to load battle box file");
        return;
    };

    sj_object_get_value_as_int(json_file2, "scale", &scale2);
    sj_object_get_value_as_int(json_file2, "spacing", &spacing2);
    sj_object_get_value_as_int(json_file2, "grid_width", &grid_width2);
    sj_object_get_value_as_int(json_file2, "grid_height", &grid_height2);

    SJson *world2 = sj_object_get_value(json_file2, "world");
    if (!world2)
    {
        slog("failed to find world in battle box config");
        sj_free(json_file2);
        return;
    }
    int tilecount2 = sj_array_get_count(world2);

    if (tilecount + tilecount2 > 1024)
    {
        slog("tilecount + tilecount2 is greater than 1024 it may crash the game");
    }
    MAIN_WORLD = gfc_allocate_array(sizeof(World), tilecount + tilecount2); // Remember entity system max 1024
    MAIN_WORLD->entityCount = tilecount + tilecount2;

    for (int i = 0; i < tilecount; i++)
    {
        SJson *tile = sj_array_get_nth(world, i);
        TextLine modelfilename;
        TextLine texturefilename;
        if (!tile)
        {
            slog("failed to find tile %d in %s world config", i, filename);
            sj_free(json_file);
            return;
        }
        const char *bottom_model_name = sj_object_get_value_as_string(tile, "bottom_model");
        if (!bottom_model_name)
        {
            slog("failed to find model in %s world config", filename);
            sj_free(json_file);
            return;
        }
        Vector3D loc;
        sj_value_as_vector3d(sj_object_get_value(tile, "location"), &loc);

        if (i == 0)
        {
            WORLD_BOUND_X1 = loc.x;
            WORLD_BOUND_Y1 = loc.y;
        }
        if (i == tilecount - 1)
        {
            WORLD_BOUND_X2 = loc.x;
            WORLD_BOUND_Y2 = loc.y;
        }

        snprintf(modelfilename, GFCLINELEN, "assets/world/%s/%s.obj", (char *)bottom_model_name, (char *)bottom_model_name);
        snprintf(texturefilename, GFCLINELEN, "assets/world/%s/%s.png", (char *)bottom_model_name, (char *)bottom_model_name);
        MAIN_WORLD[i].worldModel = gf3d_model_load_full(modelfilename, texturefilename);
        MAIN_WORLD[i].color = gfc_color(1, 1, 1, 1);
        gfc_matrix_identity(MAIN_WORLD[i].modelMat);
        gfc_matrix_scale(MAIN_WORLD[i].modelMat, vector3d(scale, scale, scale));
        gfc_matrix_translate(MAIN_WORLD[i].modelMat, loc);

        const char *top_model_name = sj_object_get_value_as_string(tile, "top_model");
        if (top_model_name)
        {

            float rotation;
            int type;
            sj_object_get_value_as_float(tile, "top_model_rotation", &rotation);
            rotation = rotation * (M_PI / 180);
            sj_object_get_value_as_int(tile, "top_model_type", &type);
            if (type == 1)
            {
                pokemon_new_name(loc, vector3d(0, 0, rotation - M_PI), top_model_name);
            }
            else if (type == 2)
            {
                interactable_new(loc, vector3d(0, 0, rotation), top_model_name);
            }
        }
    }
    sj_free(json_file);

    // load battle box
    for (int i = tilecount; i < tilecount + tilecount2; i++)
    {
        SJson *tile2 = sj_array_get_nth(world2, i - tilecount);
        TextLine modelfilename;
        TextLine texturefilename;
        if (!tile2)
        {
            slog("failed to find tile2 %d in %s world2 config", i, filename);
            sj_free(json_file);
            return;
        }
        const char *bottom_model_name2 = sj_object_get_value_as_string(tile2, "bottom_model");
        if (!bottom_model_name2)
        {
            slog("failed to find model in %s world2 config", filename);
            sj_free(json_file);
            return;
        }
        Vector3D loc2;
        sj_value_as_vector3d(sj_object_get_value(tile2, "location"), &loc2);
        snprintf(modelfilename, GFCLINELEN, "assets/world/%s/%s.obj", (char *)bottom_model_name2, (char *)bottom_model_name2);
        snprintf(texturefilename, GFCLINELEN, "assets/world/%s/%s.png", (char *)bottom_model_name2, (char *)bottom_model_name2);
        MAIN_WORLD[i].worldModel = gf3d_model_load_full(modelfilename, texturefilename);
        MAIN_WORLD[i].color = gfc_color(1, 1, 1, 1);
        gfc_matrix_identity(MAIN_WORLD[i].modelMat);
        gfc_matrix_scale(MAIN_WORLD[i].modelMat, vector3d(scale2, scale2, scale2));
        gfc_matrix_translate(MAIN_WORLD[i].modelMat, loc2);
    }

    sj_free(json_file2);

    gf3d_lights_set_global_light(vector4d(1, 1, 1, 1), vector4d(-1, -1, -1, 1));

    // Load skybox
    Vector3D skyboxScale = {1, 1, 1};
    TextLine modelfilenameSky;
    TextLine texturefilenameSky;
    snprintf(modelfilenameSky, GFCLINELEN, "assets/models/sky/skybox.obj");
    snprintf(texturefilenameSky, GFCLINELEN, "assets/models/sky/skybox-bake.png");
    MAIN_WORLD->sky = gf3d_model_load_full(modelfilenameSky, texturefilenameSky);
    gfc_matrix_identity(MAIN_WORLD->skyMat);
    gfc_matrix_scale(MAIN_WORLD->skyMat, skyboxScale);
    //shift skybox up so I can see it
}

/*eol@eof*/
