#include <stdlib.h>
#include <string.h>

#include "simple_logger.h"

#include "entity.h"
#include "gf3d_draw.h"
#include "gfc_primitives.h"

extern int __BB;
extern int SIGN_COLLISION;
extern int TREE_COLLISION;
extern int PC_COLLISION;
extern int ROCK_COLLISION;
extern int STRENGTH_COLLISION;
typedef struct
{
    Entity *entity_list;
    Uint32 entity_count;
    int current_count;
} EntityManager;

static EntityManager entity_manager = {0};

void entity_system_close()
{
    int i;
    for (i = 0; i < entity_manager.entity_count; i++)
    {
        entity_free(&entity_manager.entity_list[i]);
    }
    free(entity_manager.entity_list);
    memset(&entity_manager, 0, sizeof(EntityManager));
    slog("entity_system closed");
}

void entity_system_init(Uint32 maxEntities)
{
    entity_manager.entity_list = gfc_allocate_array(sizeof(Entity), maxEntities);
    if (entity_manager.entity_list == NULL)
    {
        slog("failed to allocate entity list, cannot allocate ZERO entities");
        return;
    }
    entity_manager.entity_count = maxEntities;
    entity_manager.current_count = 0;
    atexit(entity_system_close);
    slog("entity_system initialized");
}

Entity *entity_new()
{
    int i;
    for (i = 0; i < entity_manager.entity_count; i++)
    {
        if (!entity_manager.entity_list[i]._inuse) // not used yet, so we can!
        {
            entity_manager.current_count++;
            entity_manager.entity_list[i].entityID = entity_manager.current_count;
            entity_manager.entity_list[i]._inuse = 1;
            gfc_matrix_identity(entity_manager.entity_list[i].modelMat);
            entity_manager.entity_list[i].scale.x = 1;
            entity_manager.entity_list[i].scale.y = 1;
            entity_manager.entity_list[i].scale.z = 1;
            return &entity_manager.entity_list[i];
        }
    }
    slog("entity_new: no free space in the entity list");
    return NULL;
}

void entity_free(Entity *self)
{
    if (!self)
        return;
    // MUST DESTROY
    gf3d_model_free(self->model);
    memset(self, 0, sizeof(Entity));
}

void entity_draw(Entity *self)
{
    if (!self)
        return;
    gf3d_model_draw(self->model, self->modelMat, vector4d(1, 1, 1, 1), vector4d(0, 0, 0, 0));

    // draw bounding box if it exists
    if (__BB && self->isBox)
    {
        gf3d_draw_cube_wireframe(self->boundingBox, vector3d(self->position.x, self->position.y, self->position.z), vector3d(0, 0, self->rotation.z), vector3d(1, 1, 1), gfc_color(0, 0, 1, 1));
    }
}

void entity_draw_all()
{
    int i;
    for (i = 0; i < entity_manager.entity_count; i++)
    {
        if (!entity_manager.entity_list[i]._inuse) // not used yet
        {
            continue; // skip this iteration of the loop
        }
        entity_draw(&entity_manager.entity_list[i]);
    }
}

void entity_think(Entity *self)
{
    if (!self)
        return;
    if (self->think)
        self->think(self);
}

void entity_think_all()
{
    int i;
    for (i = 0; i < entity_manager.entity_count; i++)
    {
        if (!entity_manager.entity_list[i]._inuse) // not used yet
        {
            continue; // skip this iteration of the loop
        }
        entity_think(&entity_manager.entity_list[i]);
    }
}

void entity_update(Entity *self)
{
    if (!self)
        return;
    // HANDLE ALL COMMON UPDATE STUFF

    vector3d_add(self->position, self->position, self->velocity);
    vector3d_add(self->velocity, self->acceleration, self->velocity);

    gfc_matrix_identity(self->modelMat);
    gfc_matrix_scale(self->modelMat, self->scale);

    gfc_matrix_rotate(self->modelMat, self->modelMat, self->rotation.z, vector3d(0, 0, 1));
    gfc_matrix_rotate(self->modelMat, self->modelMat, self->rotation.y, vector3d(0, 1, 0));
    gfc_matrix_rotate(self->modelMat, self->modelMat, self->rotation.x, vector3d(1, 0, 0));

    gfc_matrix_translate(self->modelMat, self->position);

    if (self->update)
        self->update(self);
}

void entity_update_all()
{
    int i;
    for (i = 0; i < entity_manager.entity_count; i++)
    {
        if (!entity_manager.entity_list[i]._inuse) // not used yet
        {
            continue; // skip this iteration of the loop
        }
        entity_update(&entity_manager.entity_list[i]);
    }
}

void entity_collide_check(Entity *ent)
{
    int i;
    Box BoundA, BoundB;

    if (!ent)
        return;
    if (ent->type == ET_TRAINER)
    {
        BoundA = ent->boundingBox;
        vector3d_add(BoundA, BoundA, ent->position);
        for (i = 0; i < entity_manager.entity_count; i++)
        {
            if (!entity_manager.entity_list[i]._inuse || entity_manager.entity_list[i].type == ET_TRAINER) // not used yet
            {
                continue; // skip this iteration of the loop
            }
            BoundB = entity_manager.entity_list[i].boundingBox;
            vector3d_add(BoundB, BoundB, entity_manager.entity_list[i].position);
            if (gfc_box_overlap(BoundA, BoundB))
            {
                if (ent->collide)
                {
                    ent->collide(ent, &entity_manager.entity_list[i]);
                }
            }
        }
    }
}
void entity_collide_check_all()
{
    int i;
    for (i = 0; i < entity_manager.entity_count; i++)
    {
        if (!entity_manager.entity_list[i]._inuse) // not used yet
        {
            continue; // skip this iteration of the loop
        }
        entity_collide_check(&entity_manager.entity_list[i]);
    }
}

Entity *entity_get(char *name, int id)
{
    int i;
    for (i = 0; i < entity_manager.entity_count; i++)
    {
        if (!entity_manager.entity_list[i]._inuse) // not used yet
        {
            continue; // skip this iteration of the loop
        }
        if (strcmp(entity_manager.entity_list[i].name, name) == 0 && entity_manager.entity_list[i].entityID == id)
        {
            return &entity_manager.entity_list[i];
        }
    }
    return NULL;
}

/*eol@eof*/
