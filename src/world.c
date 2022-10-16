#include "simple_logger.h"
#include "simple_json.h"
#include "gfc_types.h"

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
    SJson *json, *wjson;
    World *w = NULL;
    const char *modelName = NULL;
    int scale;
    int tileCount = 100;
    w = gfc_allocate_array(sizeof(World), tileCount);
    w->tileCount = tileCount;
    if (w == NULL)
    {
        slog("failed to allocate data for the world");
        return NULL;
    }
    json = sj_load(filename);
    if (!json)
    {
        slog("failed to load json file (%s) for the world data", filename);
        free(w);
        return NULL;
    }
    wjson = sj_object_get_value(json, "world");
    if (!wjson)
    {
        slog("failed to find world object in %s world config", filename);
        free(w);
        sj_free(json);
        return NULL;
    }
    modelName = sj_get_string_value(sj_object_get_value(wjson, "model"));
    sj_get_integer_value(sj_object_get_value(wjson, "scale"), &scale);

    if (modelName)
    {

        for (int i = 0; i < tileCount / 4; i++)
        {
            w[i].worldModel = gf3d_model_load((char *)modelName);
            gfc_matrix_identity(w[i].modelMat);
            gfc_matrix_scale(w[i].modelMat, vector3d(scale, scale, scale));
            gfc_matrix_translate(w[i].modelMat, vector3d(i * 100 - (tileCount / 8 * 100), -150, 0));
        }
        for (int i = tileCount / 4; i < tileCount / 2; i++)
        {
            w[i].worldModel = gf3d_model_load((char *)modelName);
            gfc_matrix_identity(w[i].modelMat);
            gfc_matrix_scale(w[i].modelMat, vector3d(scale, scale, scale));
            gfc_matrix_translate(w[i].modelMat, vector3d((i - tileCount / 4) * 100 - (tileCount / 8 * 100), -50, 0));
        }
        for (int i = tileCount / 2; i < (tileCount / 4) + (tileCount / 2); i++)
        {
            w[i].worldModel = gf3d_model_load((char *)modelName);
            gfc_matrix_identity(w[i].modelMat);
            gfc_matrix_scale(w[i].modelMat, vector3d(scale, scale, scale));
            gfc_matrix_translate(w[i].modelMat, vector3d((i - ((tileCount / 2))) * 100 - (tileCount / 8 * 100), 50, 0));
        }
        for (int i = (tileCount / 4) + (tileCount / 2); i < tileCount; i++)
        {
            w[i].worldModel = gf3d_model_load((char *)modelName);
            gfc_matrix_identity(w[i].modelMat);
            gfc_matrix_scale(w[i].modelMat, vector3d(scale, scale, scale));
            gfc_matrix_translate(w[i].modelMat, vector3d((i - ((tileCount / 4) + (tileCount / 2))) * 100 - (tileCount / 8 * 100), 150, 0));
        }
    }
    else
    {
        slog("world data (%s) has no model", filename);
    }
    sj_free(json);
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
    for (int i = 0; i < world->tileCount; i++)
    {
        gf3d_model_draw(world[i].worldModel, world[i].modelMat);
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
