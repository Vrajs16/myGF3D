#include "simple_logger.h"
#include "gfc_primitives.h"
#include "gf3d_draw.h"
#include "trainer.h"


float TRAINER_X = 0;
float TRAINER_Y = 0;
float TRAINER_ROT_Z = 0;

void trainer_think(Entity *self);

Entity *trainer_new(Vector3D position, Vector3D rotation, char *trainer, float scale)
{
    Entity *ent = NULL;
    TextLine modelfilename;
    TextLine texturefilename;
    ent = entity_new();
    if (!ent)
    {
        slog("UGH OHHHH, no trainer for you!");
        return NULL;
    }
    snprintf(modelfilename, GFCLINELEN, "assets/trainer/%s/%s.obj", trainer, trainer);
    snprintf(texturefilename, GFCLINELEN, "assets/trainer/%s/%s-bake.png", trainer, trainer);

    ent->showBox = 1;
    ent->boundingBox = gfc_box(0,0,350,100,100,350);
    ent->name = strdup(trainer);

    ent->model = gf3d_model_load_full(modelfilename, texturefilename);
    ent->think = trainer_think;
    ent->name = strdup(trainer);
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

    if (keys[SDL_SCANCODE_D])
        rotate.z -= .08;
    if (keys[SDL_SCANCODE_A])
        rotate.z += .08;
    
    vector3d_add(self->rotation, self->rotation, rotate);
    TRAINER_ROT_Z = self->rotation.z;

    // z is up
    float yaw = self->rotation.z;

    vector3d_set(right, cos(yaw) * 30, sin(yaw) * 30, 0);
    vector3d_set(forward, -right.y, right.x, 0);
    vector3d_set(moveDir, 0, 0, 0);

    if (keys[SDL_SCANCODE_W])
        vector3d_add(moveDir, moveDir, -forward);
    else if (keys[SDL_SCANCODE_S])
        vector3d_add(moveDir, moveDir, forward);

    vector3d_add(self->position, self->position, moveDir);
    TRAINER_X = self->position.x;
    TRAINER_Y = self->position.y;
}

/*eol@eof*/
