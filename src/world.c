#include "simple_logger.h"
#include "simple_json.h"

#include "gfc_types.h"
#include "gfc_config.h"

#include "gf3d_lights.h"

#include "world.h"
/*
typedef struct
{

    Model *worldModel;
    List *spawnList;        //entities to spawn
    List *entityList;       //entities that exist in the world
}World;
*/

World *world_load(char *filename)
{
    SJson *json, *wjson, *layout, *floor, *wall, *sky;
    int floorCount;
    int wallCount;
    int skyCount;
    int scale;
    json = sj_load(filename);
    if (!json)
    {
        slog("failed to load json file (%s) for the world data", filename);
        return NULL;
    }
    wjson = sj_object_get_value(json, "world");
    if (!wjson)
    {
        slog("failed to find world object in %s world config", filename);
        sj_free(json);
        return NULL;
    }
    sj_get_integer_value(sj_object_get_value(wjson, "scale"), &scale);

    layout = sj_object_get_value(wjson, "layout");
    if (!layout)
    {
        slog("failed to find layout in %s world config", filename);
        sj_free(json);
        return NULL;
    }
    floor = sj_object_get_value(layout, "floor");
    if (!floor)
    {
        slog("failed to find floor in %s world config", filename);
        sj_free(json);
        return NULL;
    }
    floorCount = sj_array_get_count(floor);
    wall = sj_object_get_value(layout, "wall");
    if (!wall)
    {
        slog("failed to find wall in %s world config", filename);
        sj_free(json);
        return NULL;
    }
    wallCount = sj_array_get_count(wall);
    sky = sj_object_get_value(layout, "sky");
    if (!sky)
    {
        slog("failed to find sky in %s world config", filename);
        sj_free(json);
        return NULL;
    }
    skyCount = sj_array_get_count(sky);
    World *w = NULL;
    w = gfc_allocate_array(sizeof(World), floorCount + wallCount + skyCount); // Remember entity system max 1024
    w->entityCount = floorCount + wallCount + skyCount;
    if (w == NULL)
    {
        slog("failed to allocate data for the world");
        free(w);
        return NULL;
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
                free(w);
                sj_free(json);
                return NULL;
            }
            const char *model = sj_object_get_value_as_string(floorTileInfo, "model");
            sj_value_as_vector3d(sj_object_get_value(floorTileInfo, "location"), &loc);
            if (!model)
            {
                slog("failed to find model in %s world config", modelfilename);
                free(w);
                sj_free(json);
                return NULL;
            }
            snprintf(modelfilename, GFCLINELEN, "assets/world/%s/%s.obj", (char *)model, (char *)model);
            snprintf(texturefilename, GFCLINELEN, "assets/world/%s/%s.png", (char *)model, (char *)model);
            w[i].worldModel = gf3d_model_load_full(modelfilename, texturefilename);
            w[i].color = gfc_color(1, 1, 1, 1);
            gfc_matrix_identity(w[i].modelMat);
            gfc_matrix_scale(w[i].modelMat, vector3d(scale, scale, scale));
            gfc_matrix_translate(w[i].modelMat, loc);
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
                free(w);
                sj_free(json);
                return NULL;
            }
            const char *model = sj_object_get_value_as_string(wallTileInfo, "model");
            sj_value_as_vector3d(sj_object_get_value(wallTileInfo, "location"), &loc);
            if (!model)
            {
                slog("failed to find model in %s world config", modelfilename);
                free(w);
                sj_free(json);
                return NULL;
            }

            SJson *rotationInfo = sj_object_get_value(wallTileInfo, "rotation");
            if (!rotationInfo)
            {
                slog("failed to find rotation in %s world config", modelfilename);
                free(w);
                sj_free(json);
                return NULL;
            }
            Vector3D axisVector;
            int angle;
            sj_value_as_vector3d(sj_object_get_value(rotationInfo, "axis"), &axisVector);
            sj_get_integer_value(sj_object_get_value(rotationInfo, "angle"), &angle);

            snprintf(modelfilename, GFCLINELEN, "assets/world/%s/%s.obj", (char *)model, (char *)model);
            snprintf(texturefilename, GFCLINELEN, "assets/world/%s/%s.png", (char *)model, (char *)model);
            w[i].worldModel = gf3d_model_load_full(modelfilename, texturefilename);
            w[i].color = gfc_color(1, 1, 1, 1);
            gfc_matrix_identity(w[i].modelMat);
            gfc_matrix_scale(w[i].modelMat, vector3d(scale, scale, scale));

            if (angle == 1)
            {
                gfc_matrix_rotate(w[i].modelMat, w[i].modelMat, M_PI / 2, axisVector);
                gfc_matrix_translate(w[i].modelMat, vector3d(loc.x, loc.y + (scale * 100 / 2), loc.z + (scale * 100 / 2)));
            }
            else if (angle == 2)
            {
                gfc_matrix_rotate(w[i].modelMat, w[i].modelMat, -M_PI / 2, axisVector);
                gfc_matrix_translate(w[i].modelMat, vector3d(loc.x + (scale * 100 / 2), loc.y, loc.z + (scale * 100 / 2)));
            }
            else if (angle == 3)
            {
                gfc_matrix_rotate(w[i].modelMat, w[i].modelMat, 3 * M_PI / 2, axisVector);
                gfc_matrix_translate(w[i].modelMat, vector3d(loc.x, loc.y - (scale * 100 / 2), loc.z + (scale * 100 / 2)));
            }
            else if (angle == 4)
            {
                gfc_matrix_rotate(w[i].modelMat, w[i].modelMat, -3 * M_PI / 2, axisVector);
                gfc_matrix_translate(w[i].modelMat, vector3d(loc.x - (scale * 100 / 2), loc.y, loc.z + (scale * 100 / 2)));
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
                free(w);
                sj_free(json);
                return NULL;
            }
            const char *model = sj_object_get_value_as_string(skyTileInfo, "model");
            sj_value_as_vector3d(sj_object_get_value(skyTileInfo, "location"), &loc);
            if (!model)
            {
                slog("failed to find model in %s world config", modelfilename);
                free(w);
                sj_free(json);
                return NULL;
            }
            snprintf(modelfilename, GFCLINELEN, "assets/world/%s/%s.obj", (char *)model, (char *)model);
            snprintf(texturefilename, GFCLINELEN, "assets/world/%s/%s.png", (char *)model, (char *)model);
            w[i].worldModel = gf3d_model_load_full(modelfilename, texturefilename);
            w[i].color = gfc_color(1, 1, 1, 1);
            gfc_matrix_identity(w[i].modelMat);
            gfc_matrix_scale(w[i].modelMat, vector3d(scale, scale, scale));
            gfc_matrix_rotate(w[i].modelMat, w[i].modelMat, M_PI, vector3d(1, 0, 0));
            gfc_matrix_translate(w[i].modelMat, loc);
        }
    }
    else
    {
        slog("world data (%s) has no model", filename);
    }
    sj_free(json);
    gf3d_lights_set_global_light(vector4d(1, 1, 1, 1), vector4d(-1, -1, 0, 1));
    return w;
}

void world_draw(World *world)
{
    if (!world)
        return;
    if (!world->worldModel)
    {
        slog("world has no model");
        return; // no model to draw, do nothing
    }
    for (int i = 0; i < world->entityCount; i++)
    {
        gf3d_model_draw(world[i].worldModel, world[i].modelMat, gfc_color_to_vector4f(world[i].color), vector4d(0, 0, 0, 0));
    }
}

void world_delete(World *world)
{
    if (!world)
        return;
    gf3d_model_free(world->worldModel);
    free(world);
}

void world_run_updates(World *world);

void world_add_entity(World *world, Entity *entity);

/*eol@eof*/
