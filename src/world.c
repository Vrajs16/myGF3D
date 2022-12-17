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

void world_load(char *filename)
{
    return;
    SJson *json, *wjson, *layout, *floor, *wall, *sky;
    int floorCount;
    int wallCount;
    int skyCount;
    int scale;
    json = sj_load(filename);
    if (!json)
    {
        slog("failed to load json file (%s) for the world data", filename);
        return;
    }
    wjson = sj_object_get_value(json, "world");
    if (!wjson)
    {
        slog("failed to find world object in %s world config", filename);
        sj_free(json);
        return;
    }
    sj_get_integer_value(sj_object_get_value(wjson, "scale"), &scale);

    layout = sj_object_get_value(wjson, "layout");
    if (!layout)
    {
        slog("failed to find layout in %s world config", filename);
        sj_free(json);
        return;
    }
    floor = sj_object_get_value(layout, "floor");
    if (!floor)
    {
        slog("failed to find floor in %s world config", filename);
        sj_free(json);
        return;
    }
    floorCount = sj_array_get_count(floor);
    wall = sj_object_get_value(layout, "wall");
    if (!wall)
    {
        slog("failed to find wall in %s world config", filename);
        sj_free(json);
        return;
    }
    wallCount = sj_array_get_count(wall);
    sky = sj_object_get_value(layout, "sky");
    if (!sky)
    {
        slog("failed to find sky in %s world config", filename);
        sj_free(json);
        return;
    }
    skyCount = sj_array_get_count(sky);

    // Load Battle box
    SJson *jsonB, *wjsonB, *layoutB, *floorB, *wallB, *skyB;
    int floorCountB;
    int wallCountB;
    int skyCountB;
    int scaleB;

    jsonB = sj_load("config/battle-box.json");
    if (!jsonB)
    {
        slog("failed to load json file (%s) for the world data", filename);
        return;
    }
    wjsonB = sj_object_get_value(jsonB, "world");
    if (!wjsonB)
    {
        slog("failed to find world object in %s world config", filename);
        sj_free(jsonB);
        return;
    }
    sj_get_integer_value(sj_object_get_value(wjsonB, "scale"), &scaleB);

    layoutB = sj_object_get_value(wjsonB, "layout");
    if (!layoutB)
    {
        slog("failed to find layout in %s world config", filename);
        sj_free(jsonB);
        return;
    }
    floorB = sj_object_get_value(layoutB, "floor");
    if (!floorB)
    {
        slog("failed to find floor in %s world config", filename);
        sj_free(jsonB);
        return;
    }
    floorCountB = sj_array_get_count(floorB);
    wallB = sj_object_get_value(layoutB, "wall");
    if (!wallB)
    {
        slog("failed to find wall in %s world config", filename);
        sj_free(jsonB);
        return;
    }
    wallCountB = sj_array_get_count(wallB);
    skyB = sj_object_get_value(layoutB, "sky");
    if (!skyB)
    {
        slog("failed to find sky in %s world config", filename);
        sj_free(jsonB);
        return;
    }
    skyCountB = sj_array_get_count(skyB);

    // Each world tile is its own model
    MAIN_WORLD = gfc_allocate_array(sizeof(World), floorCount + wallCount + skyCount + floorCountB + wallCountB + skyCountB); // Remember entity system max 1024
    MAIN_WORLD->entityCount = floorCount + wallCount + skyCount + floorCountB + wallCountB + skyCountB;
    if (!MAIN_WORLD)
    {
        slog("failed to allocate data for the world");
        free(MAIN_WORLD);
        return;
    }

    if (floor)
    {
        for (int i = 0; i < floorCount; i++)
        {
            Vector3D loc;
            TextLine modelfilename;
            TextLine texturefilename;
            SJson *floorTileInfo = sj_array_get_nth(floor, i);
            if (!floorTileInfo)
            {
                slog("failed to find floorTileInfo in %s world config", modelfilename);
                free(MAIN_WORLD);
                sj_free(json);
                return;
            }
            const char *model = sj_object_get_value_as_string(floorTileInfo, "model");
            sj_value_as_vector3d(sj_object_get_value(floorTileInfo, "location"), &loc);
            if (!model)
            {
                slog("failed to find model in %s world config", modelfilename);
                free(MAIN_WORLD);
                sj_free(json);
                return;
            }
            snprintf(modelfilename, GFCLINELEN, "assets/world/%s/%s.obj", (char *)model, (char *)model);
            snprintf(texturefilename, GFCLINELEN, "assets/world/%s/%s.png", (char *)model, (char *)model);
            MAIN_WORLD[i].worldModel = gf3d_model_load_full(modelfilename, texturefilename);
            MAIN_WORLD[i].color = gfc_color(1, 1, 1, 1);
            gfc_matrix_identity(MAIN_WORLD[i].modelMat);
            gfc_matrix_scale(MAIN_WORLD[i].modelMat, vector3d(scale, scale, scale));
            gfc_matrix_translate(MAIN_WORLD[i].modelMat, loc);
        }

        for (int i = floorCount; i < floorCount + wallCount; i++)
        {
            Vector3D loc;
            TextLine modelfilename;
            TextLine texturefilename;
            SJson *wallTileInfo = sj_array_get_nth(wall, i - floorCount);
            if (!wallTileInfo)
            {
                slog("failed to find wallTileInfo in %s world config", modelfilename);
                free(MAIN_WORLD);
                sj_free(json);
                return;
            }
            const char *model = sj_object_get_value_as_string(wallTileInfo, "model");
            sj_value_as_vector3d(sj_object_get_value(wallTileInfo, "location"), &loc);
            if (!model)
            {
                slog("failed to find model in %s world config", modelfilename);
                free(MAIN_WORLD);
                sj_free(json);
                return;
            }

            SJson *rotationInfo = sj_object_get_value(wallTileInfo, "rotation");
            if (!rotationInfo)
            {
                slog("failed to find rotation in %s world config", modelfilename);
                free(MAIN_WORLD);
                sj_free(json);
                return;
            }
            Vector3D axisVector;
            int angle;
            sj_value_as_vector3d(sj_object_get_value(rotationInfo, "axis"), &axisVector);
            sj_get_integer_value(sj_object_get_value(rotationInfo, "angle"), &angle);

            snprintf(modelfilename, GFCLINELEN, "assets/world/%s/%s.obj", (char *)model, (char *)model);
            snprintf(texturefilename, GFCLINELEN, "assets/world/%s/%s.png", (char *)model, (char *)model);
            MAIN_WORLD[i].worldModel = gf3d_model_load_full(modelfilename, texturefilename);
            MAIN_WORLD[i].color = gfc_color(1, 1, 1, 1);
            gfc_matrix_identity(MAIN_WORLD[i].modelMat);
            gfc_matrix_scale(MAIN_WORLD[i].modelMat, vector3d(scale, scale, scale));

            if (angle == 1)
            {
                gfc_matrix_rotate(MAIN_WORLD[i].modelMat, MAIN_WORLD[i].modelMat, M_PI / 2, axisVector);
                gfc_matrix_translate(MAIN_WORLD[i].modelMat, vector3d(loc.x, loc.y + (scale * 100 / 2), loc.z + (scale * 100 / 2)));
            }
            else if (angle == 2)
            {
                gfc_matrix_rotate(MAIN_WORLD[i].modelMat, MAIN_WORLD[i].modelMat, -M_PI / 2, axisVector);
                gfc_matrix_translate(MAIN_WORLD[i].modelMat, vector3d(loc.x + (scale * 100 / 2), loc.y, loc.z + (scale * 100 / 2)));
            }
            else if (angle == 3)
            {
                gfc_matrix_rotate(MAIN_WORLD[i].modelMat, MAIN_WORLD[i].modelMat, 3 * M_PI / 2, axisVector);
                gfc_matrix_translate(MAIN_WORLD[i].modelMat, vector3d(loc.x, loc.y - (scale * 100 / 2), loc.z + (scale * 100 / 2)));
            }
            else if (angle == 4)
            {
                gfc_matrix_rotate(MAIN_WORLD[i].modelMat, MAIN_WORLD[i].modelMat, -3 * M_PI / 2, axisVector);
                gfc_matrix_translate(MAIN_WORLD[i].modelMat, vector3d(loc.x - (scale * 100 / 2), loc.y, loc.z + (scale * 100 / 2)));
            }
            else
            {
                slog("ENTERED WRONG ANGGLE!, FIX THIS");
            }
        }

        for (int i = floorCount + wallCount; i < floorCount + wallCount + skyCount; i++)
        {
            Vector3D loc;
            TextLine modelfilename;
            TextLine texturefilename;
            SJson *skyTileInfo = sj_array_get_nth(sky, i - floorCount - wallCount);
            if (!skyTileInfo)
            {
                slog("failed to find skyTileInfo in %s world config", modelfilename);
                free(MAIN_WORLD);
                sj_free(json);
                return;
            }
            const char *model = sj_object_get_value_as_string(skyTileInfo, "model");
            sj_value_as_vector3d(sj_object_get_value(skyTileInfo, "location"), &loc);
            if (!model)
            {
                slog("failed to find model in %s world config", modelfilename);
                free(MAIN_WORLD);
                sj_free(json);
                return;
            }
            snprintf(modelfilename, GFCLINELEN, "assets/world/%s/%s.obj", (char *)model, (char *)model);
            snprintf(texturefilename, GFCLINELEN, "assets/world/%s/%s.png", (char *)model, (char *)model);
            MAIN_WORLD[i].worldModel = gf3d_model_load_full(modelfilename, texturefilename);
            MAIN_WORLD[i].color = gfc_color(1, 1, 1, 1);
            gfc_matrix_identity(MAIN_WORLD[i].modelMat);
            gfc_matrix_scale(MAIN_WORLD[i].modelMat, vector3d(scale, scale, scale));
            gfc_matrix_rotate(MAIN_WORLD[i].modelMat, MAIN_WORLD[i].modelMat, M_PI, vector3d(1, 0, 0));
            gfc_matrix_translate(MAIN_WORLD[i].modelMat, loc);
        }

        // Loading Battle Arena
        for (int i = floorCount + wallCount + skyCount; i < floorCount + wallCount + skyCount + floorCountB; i++)
        {
            Vector3D loc;
            TextLine modelfilename;
            TextLine texturefilename;
            SJson *floorTileInfo = sj_array_get_nth(floorB, i - floorCount - wallCount - skyCount);
            if (!floorTileInfo)
            {
                slog("failed to find floorTileInfo in %s world config", modelfilename);
                free(MAIN_WORLD);
                sj_free(json);
                return;
            }
            const char *model = sj_object_get_value_as_string(floorTileInfo, "model");
            sj_value_as_vector3d(sj_object_get_value(floorTileInfo, "location"), &loc);
            if (!model)
            {
                slog("failed to find model in %s world config", modelfilename);
                free(MAIN_WORLD);
                sj_free(json);
                return;
            }
            snprintf(modelfilename, GFCLINELEN, "assets/world/%s/%s.obj", (char *)model, (char *)model);
            snprintf(texturefilename, GFCLINELEN, "assets/world/%s/%s.png", (char *)model, (char *)model);
            MAIN_WORLD[i].worldModel = gf3d_model_load_full(modelfilename, texturefilename);
            MAIN_WORLD[i].color = gfc_color(1, 1, 1, 1);
            gfc_matrix_identity(MAIN_WORLD[i].modelMat);
            gfc_matrix_scale(MAIN_WORLD[i].modelMat, vector3d(scaleB, scaleB, scaleB));
            gfc_matrix_translate(MAIN_WORLD[i].modelMat, loc);
        }

        for (int i = floorCount + wallCount + skyCount + floorCountB; i < floorCount + wallCount + skyCount + floorCountB + wallCountB; i++)
        {
            Vector3D loc;
            TextLine modelfilename;
            TextLine texturefilename;
            SJson *wallTileInfo = sj_array_get_nth(wallB, i - floorCount - wallCount - skyCount - floorCountB);
            if (!wallTileInfo)
            {
                slog("failed to find wallTileInfo in %s world config", modelfilename);
                free(MAIN_WORLD);
                sj_free(json);
                return;
            }
            const char *model = sj_object_get_value_as_string(wallTileInfo, "model");
            sj_value_as_vector3d(sj_object_get_value(wallTileInfo, "location"), &loc);
            if (!model)
            {
                slog("failed to find model in %s world config", modelfilename);
                free(MAIN_WORLD);
                sj_free(json);
                return;
            }

            SJson *rotationInfo = sj_object_get_value(wallTileInfo, "rotation");
            if (!rotationInfo)
            {
                slog("failed to find rotation in %s world config", modelfilename);
                free(MAIN_WORLD);
                sj_free(json);
                return;
            }
            Vector3D axisVector;
            int angle;
            sj_value_as_vector3d(sj_object_get_value(rotationInfo, "axis"), &axisVector);
            sj_get_integer_value(sj_object_get_value(rotationInfo, "angle"), &angle);

            snprintf(modelfilename, GFCLINELEN, "assets/world/%s/%s.obj", (char *)model, (char *)model);
            snprintf(texturefilename, GFCLINELEN, "assets/world/%s/%s.png", (char *)model, (char *)model);
            MAIN_WORLD[i].worldModel = gf3d_model_load_full(modelfilename, texturefilename);
            MAIN_WORLD[i].color = gfc_color(1, 1, 1, 1);
            gfc_matrix_identity(MAIN_WORLD[i].modelMat);
            gfc_matrix_scale(MAIN_WORLD[i].modelMat, vector3d(scaleB, scaleB, scaleB));

            if (angle == 1)
            {
                gfc_matrix_rotate(MAIN_WORLD[i].modelMat, MAIN_WORLD[i].modelMat, M_PI / 2, axisVector);
                gfc_matrix_translate(MAIN_WORLD[i].modelMat, vector3d(loc.x, loc.y + (scaleB * 100 / 2), loc.z + (scaleB * 100 / 2)));
            }
            else if (angle == 2)
            {
                gfc_matrix_rotate(MAIN_WORLD[i].modelMat, MAIN_WORLD[i].modelMat, -M_PI / 2, axisVector);
                gfc_matrix_translate(MAIN_WORLD[i].modelMat, vector3d(loc.x + (scaleB * 100 / 2), loc.y, loc.z + (scaleB * 100 / 2)));
            }
            else if (angle == 3)
            {
                gfc_matrix_rotate(MAIN_WORLD[i].modelMat, MAIN_WORLD[i].modelMat, 3 * M_PI / 2, axisVector);
                gfc_matrix_translate(MAIN_WORLD[i].modelMat, vector3d(loc.x, loc.y - (scaleB * 100 / 2), loc.z + (scaleB * 100 / 2)));
            }
            else if (angle == 4)
            {
                gfc_matrix_rotate(MAIN_WORLD[i].modelMat, MAIN_WORLD[i].modelMat, -3 * M_PI / 2, axisVector);
                gfc_matrix_translate(MAIN_WORLD[i].modelMat, vector3d(loc.x - (scaleB * 100 / 2), loc.y, loc.z + (scaleB * 100 / 2)));
            }
            else
            {
                slog("ENTERED WRONG ANGGLE!, FIX THIS");
            }
        }

        for (int i = floorCount + wallCount + skyCount + floorCountB + wallCountB; i < floorCount + wallCount + skyCount + floorCountB + wallCountB + skyCountB; i++)
        {
            Vector3D loc;
            TextLine modelfilename;
            TextLine texturefilename;
            SJson *skyTileInfo = sj_array_get_nth(skyB, i - floorCount - wallCount - skyCount - floorCountB - wallCountB);
            if (!skyTileInfo)
            {
                slog("failed to find skyTileInfo in %s world config", modelfilename);
                free(MAIN_WORLD);
                sj_free(json);
                return;
            }
            const char *model = sj_object_get_value_as_string(skyTileInfo, "model");
            sj_value_as_vector3d(sj_object_get_value(skyTileInfo, "location"), &loc);
            if (!model)
            {
                slog("failed to find model in %s world config", modelfilename);
                free(MAIN_WORLD);
                sj_free(json);
                return;
            }
            snprintf(modelfilename, GFCLINELEN, "assets/world/%s/%s.obj", (char *)model, (char *)model);
            snprintf(texturefilename, GFCLINELEN, "assets/world/%s/%s.png", (char *)model, (char *)model);
            MAIN_WORLD[i].worldModel = gf3d_model_load_full(modelfilename, texturefilename);
            MAIN_WORLD[i].color = gfc_color(1, 1, 1, 1);
            gfc_matrix_identity(MAIN_WORLD[i].modelMat);
            gfc_matrix_scale(MAIN_WORLD[i].modelMat, vector3d(scaleB, scaleB, scaleB));
            gfc_matrix_rotate(MAIN_WORLD[i].modelMat, MAIN_WORLD[i].modelMat, M_PI, vector3d(1, 0, 0));
            gfc_matrix_translate(MAIN_WORLD[i].modelMat, loc);
        }
    }
    else
    {
        slog("world data (%s) has no model", filename);
    }
    sj_free(json);
    gf3d_lights_set_global_light(vector4d(1, 1, 1, 1), vector4d(-1, -1, 0, 1));
}

void world_draw()
{
    if (!MAIN_WORLD)
        return;
    if (!MAIN_WORLD->worldModel)
    {
        slog("world has no model");
        return; // no model to draw, do nothing
    }
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
    int worldCount = sj_array_get_count(world);
    slog("worldCount: %i", worldCount);

    if (worldCount > 1024)
    {
        slog("worldCount is greater than 1024 it may crash the game");
    }
    MAIN_WORLD = gfc_allocate_array(sizeof(World), worldCount); // Remember entity system max 1024
    MAIN_WORLD->entityCount = worldCount;

    for (int i = 0; i < worldCount; i++)
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
    gf3d_lights_set_global_light(vector4d(1, 1, 1, 1), vector4d(-1, -1, 0, 1));
}

/*eol@eof*/
