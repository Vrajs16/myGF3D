
#include "simple_logger.h"
#include "trainer.h"

float TRAINER_X = 0;
float TRAINER_Y = 0;

void trainer_think(Entity *self);

Entity *trainer_new(Vector3D position, Vector3D rotation, char *trainer, float scale)
{
    Entity *ent = NULL;

    ent = entity_new();
    if (!ent)
    {
        slog("UGH OHHHH, no trainer for you!");
        return NULL;
    }

    ent->model = gf3d_model_load(trainer, 0);
    ent->think = trainer_think;
    vector3d_copy(ent->scale, vector3d(scale, scale, scale));
    vector3d_copy(ent->rotation, rotation);
    vector3d_copy(ent->position, position);
    return ent;
}

void trainer_think(Entity *self)
{
    if (!self)
        return;
    int isForward = 0;
    int isBackward = 0;
    int isLeft = 0;
    int isRight = 0;
    Vector3D rotate = {0};
    Vector3D forward = {0};
    Vector3D right = {0};
    Vector3D moveDir = {0};
    const Uint8 *keys;
    keys = SDL_GetKeyboardState(NULL); // get the keyboard state for this frame

    if (keys[SDL_SCANCODE_RIGHT])
    {
        rotate.z -= .02;
    }

    if (keys[SDL_SCANCODE_LEFT])
    {
        rotate.z += .02;
    }
    vector3d_add(self->rotation, self->rotation, rotate);

    // z is up
    float yaw = self->rotation.z;

    vector3d_set(right, cos(yaw) * 5, sin(yaw) * 5, 0);
    vector3d_set(forward, -right.y, right.x, 0);
    vector3d_set(moveDir, 0, 0, 0);

    if (keys[SDL_SCANCODE_W])
    {
        isForward = 1;
        vector3d_add(moveDir, moveDir, -forward);
    }
    else if (keys[SDL_SCANCODE_S])
    {
        isBackward = 1;
        vector3d_add(moveDir, moveDir, forward);
    }
    if (keys[SDL_SCANCODE_D])
    {
        isRight = 1;
        vector3d_add(moveDir, moveDir, -right);
    }
    else if (keys[SDL_SCANCODE_A])
    {
        isLeft = 1;
        vector3d_add(moveDir, moveDir, right);
    }

    if (isForward && isRight)
    {
        self->rotation.y = -.2;
        self->rotation.x = .4;
    }

    if (isForward && isLeft)
    {
        self->rotation.y = .2;
        self->rotation.x = .4;
    }

    if (isBackward && isRight)
    {
        self->rotation.y = -.2;
        self->rotation.x = -.4;
    }

    if (isBackward && isLeft)
    {
        self->rotation.y = .2;
        self->rotation.x = -.4;
    }

    if (isForward && !isRight && !isLeft)
    {
        self->rotation.y = 0;
        self->rotation.x = .4;
    }

    if (isBackward && !isRight && !isLeft)
    {
        self->rotation.y = 0;
        self->rotation.x = -.4;
    }

    if (isRight && !isForward && !isBackward)
    {
        self->rotation.y = -.2;
        self->rotation.x = 0;
    }

    if (isLeft && !isForward && !isBackward)
    {
        self->rotation.y = .2;
        self->rotation.x = 0;
    }

    if (!isForward && !isBackward && !isRight && !isLeft)
    {
        self->rotation.y = 0;
        self->rotation.x = 0;
    }

    vector3d_add(self->position, self->position, moveDir);
    TRAINER_X = self->position.x;
    TRAINER_Y = self->position.y;
}

/*eol@eof*/
