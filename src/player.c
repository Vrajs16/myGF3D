#include "simple_logger.h"
#include "gfc_types.h"

#include "gf3d_camera.h"
#include "player.h"

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
    ent->rotation.x = -M_PI;
    return ent;
}

void player_think(Entity *self)
{
    Vector3D forward;
    Vector3D right;
    Vector3D up;
    Vector3D moveDir;
    const Uint8 *keys;
    keys = SDL_GetKeyboardState(NULL); // get the keyboard state for this frame

    if (keys[SDL_SCANCODE_RIGHT])
        self->rotation.z -= 0.0075;
    if (keys[SDL_SCANCODE_LEFT])
        self->rotation.z += 0.0075;
    if (keys[SDL_SCANCODE_UP])
        self->rotation.x -= 0.0075;
    if (keys[SDL_SCANCODE_DOWN])
        self->rotation.x += 0.0075;

    // z is up
    float yaw = self->rotation.z;

    vector3d_set(right, cos(yaw), sin(yaw), 0);
    vector3d_set(forward, -right.y, right.x, 0);
    vector3d_set(up, 0, 0, 1);
    vector3d_set(moveDir, 0, 0, 0);

    if (keys[SDL_SCANCODE_W])
        vector3d_add(moveDir, moveDir, forward);
    if (keys[SDL_SCANCODE_S])
        vector3d_add(moveDir, moveDir, -forward);
    if (keys[SDL_SCANCODE_D])
        vector3d_add(moveDir, moveDir, right);
    if (keys[SDL_SCANCODE_A])
        vector3d_add(moveDir, moveDir, -right);
    if (keys[SDL_SCANCODE_SPACE])
        vector3d_add(moveDir, moveDir, up);
    if (keys[SDL_SCANCODE_LSHIFT])
        vector3d_add(moveDir, moveDir, -up);

    vector3d_normalize(&moveDir);
    vector3d_add(self->position, self->position, moveDir);
}

void player_update(Entity *self)
{
    if (!self)
        return;
    gf3d_camera_set_position(self->position);
    gf3d_camera_set_rotation(self->rotation);
}

/*eol@eof*/
