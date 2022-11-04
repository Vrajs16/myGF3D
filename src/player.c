#include "simple_logger.h"
#include "gfc_types.h"

#include "gf3d_camera.h"
#include "player.h"

extern float TRAINER_X;
extern float TRAINER_Y;
extern float TRAINER_ROT_Z;

void player_think(Entity *self);
void player_update(Entity *self);

Entity *player_new(Vector3D position)
{
    Entity *ent = NULL;

    ent = entity_new();
    if (!ent)
    {
        slog("UGH OHHHH, no player for you!");
        return NULL;
    }
    ent->name = "camera";
    ent->think = player_think;
    ent->update = player_update;
    vector3d_copy(ent->position, position);
    ent->rotation.x = M_PI + .2;
    return ent;
}

void player_think(Entity *self)
{
    if (!self)
        return;
    vector3d_set(self->rotation, M_PI + .2,0,TRAINER_ROT_Z-M_PI);
    float x = 1800 * cos(self->rotation.z - M_PI_2) + TRAINER_X;
    float y = 1800 * sin(self->rotation.z - M_PI_2) + TRAINER_Y;
    vector3d_set(self->position, x, y, 1000);
}

void player_update(Entity *self)
{
    if (!self)
        return;
    gf3d_camera_set_position(self->position);
    gf3d_camera_set_rotation(self->rotation);
}

/*eol@eof*/
