#include "simple_logger.h"
#include "gfc_types.h"

#include "gf3d_camera.h"
#include "player.h"

extern float TRAINER_X;
extern float TRAINER_Y;
extern float TRAINER_Z;
extern float TRAINER_ROT_Z;

extern int BATTLE;

int COUNT = 0;

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
    ent->rotation.z = TRAINER_ROT_Z - M_PI;
    return ent;
}

void player_think(Entity *self)
{
    if (!self)
        return;

    Vector3D finalRotation = vector3d(M_PI + .2, 0, TRAINER_ROT_Z - M_PI);
    Vector3D nextRotation = vector3d(0, 0, 0);
    nextRotation.x = self->rotation.x + (finalRotation.x - self->rotation.x) * .05;
    nextRotation.y = self->rotation.y + (finalRotation.y - self->rotation.y) * .05;
    nextRotation.z = self->rotation.z + (finalRotation.z - self->rotation.z) * .05;

    vector3d_copy(self->rotation, nextRotation);
    if (BATTLE)
    {
        vector3d_copy(self->rotation, finalRotation);
        COUNT = 1;
    }
    if (COUNT && !BATTLE)
    {
        vector3d_copy(self->rotation, finalRotation);
        COUNT = 0;
    }
    Vector3D finalPos = vector3d(1800 * cos(self->rotation.z - M_PI_2) + TRAINER_X, 1800 * sin(self->rotation.z - M_PI_2) + TRAINER_Y, TRAINER_Z);
    vector3d_copy(self->position, finalPos);
}

void player_update(Entity *self)
{
    if (!self)
        return;
    gf3d_camera_set_position(self->position);
    gf3d_camera_set_rotation(self->rotation);
}

/*eol@eof*/
