#include "simple_logger.h"
#include "gfc_types.h"

#include "gf3d_camera.h"
#include "player.h"

extern float TRAINER_X;
extern float TRAINER_Y;

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

    //    ent->model = gf3d_model_load("dino");
    ent->think = player_think;
    ent->update = player_update;
    vector3d_copy(ent->position, position);
    ent->rotation.x = M_PI + .4;
    return ent;
}

void player_think(Entity *self)
{
    if (!self)
        return;

    Vector3D rotate = {0};
    Vector3D forward = {0};
    Vector3D right = {0};
    Vector3D moveDir = {0};

    const Uint8 *keys;
    keys = SDL_GetKeyboardState(NULL); // get the keyboard state for this frame
    if (keys[SDL_SCANCODE_RIGHT])
        rotate.z += .02;
    if (keys[SDL_SCANCODE_LEFT])
        rotate.z -= .02;

    vector3d_sub(self->rotation, self->rotation, rotate);

    // z is up
    float yaw = self->rotation.z;

    vector3d_set(right, cos(yaw) * 5, sin(yaw) * 5, 0);
    vector3d_set(forward, -right.y, right.x, 0);
    vector3d_set(moveDir, 0, 0, 0);

    if (keys[SDL_SCANCODE_W])
        vector3d_add(moveDir, moveDir, forward);
    else if (keys[SDL_SCANCODE_S])
        vector3d_add(moveDir, moveDir, -forward);
    if (keys[SDL_SCANCODE_D])
        vector3d_add(moveDir, moveDir, right);
    else if (keys[SDL_SCANCODE_A])
        vector3d_add(moveDir, moveDir, -right);

    vector3d_add(self->position, self->position, moveDir);

    float x = 1300 * cos(yaw - M_PI_2) + TRAINER_X;
    float y = 1300 * sin(yaw - M_PI_2) + TRAINER_Y;
    vector3d_set(self->position, x, y, 600);
}

void player_update(Entity *self)
{
    if (!self)
        return;
    gf3d_camera_set_position(self->position);
    gf3d_camera_set_rotation(self->rotation);
}

/*eol@eof*/
