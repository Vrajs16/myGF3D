#ifndef __WORLD_H__
#define __WORLD_H__

#include "gfc_types.h"
#include "gfc_list.h"
#include "gfc_color.h"
#include "gfc_matrix.h"

#include "gf3d_vgraphics.h"
#include "gf3d_model.h"

#include "entity.h"

typedef struct
{
    Matrix4 modelMat;
    Model *worldModel;
    int entityCount;
    Color color;
} World;

void world_draw();

void world_delete();

void world_run_updates(World *world);

void world_add_entity(World *world, Entity *entity);

void world_load_json(char *filename);

#endif
