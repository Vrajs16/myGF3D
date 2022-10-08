
#include "simple_logger.h"
#include "agumon.h"

void agumon_think(Entity *self);

Entity *agumon_new(Vector3D position, Vector3D rotation)
{
    Entity *ent = NULL;

    ent = entity_new();
    if (!ent)
    {
        slog("UGH OHHHH, no agumon for you!");
        return NULL;
    }

    ent->model = gf3d_model_load("tyranitar");
    // ent->think = agumon_think;

    vector3d_copy(ent->scale, vector3d(10, 10, 10));
    vector3d_copy(ent->rotation, rotation);
    vector3d_copy(ent->position, position);
    return ent;
}

void agumon_think(Entity *self)
{
    if (!self)
        return;
    self->rotation.x += -0.002;
}

/*eol@eof*/
