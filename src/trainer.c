#include "simple_logger.h"
#include "gfc_primitives.h"
#include "gf3d_draw.h"
#include "trainer.h"

float TRAINER_X = 0;
float TRAINER_Y = 0;
float TRAINER_ROT_Z = 0;

void trainer_think(Entity *self);
void trainer_collide(Entity *self, Entity *other);

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

    ent->isBox = 1;
    ent->boundingBox = gfc_box(0, 0, 350, 100, 100, 350);

    ent->model = gf3d_model_load_full(modelfilename, texturefilename);
    ent->think = trainer_think;
    ent->collide = trainer_collide;
    ent->name = trainer;
    ent->type = ET_TRAINER;
    vector3d_copy(ent->scale, vector3d(scale, scale, scale));
    vector3d_copy(ent->rotation, rotation);
    vector3d_copy(ent->previousPosition, position);
    vector3d_copy(ent->position, position);
    return ent;
}

void trainer_think(Entity *self)
{
    if (!self)
        return;
    Vector3D rotate = {0};
    Vector3D forward = {0};
    Vector3D moveDir = {0};
    const Uint8 *keys;
    keys = SDL_GetKeyboardState(NULL); // get the keyboard state for this frame

    if (keys[SDL_SCANCODE_D])
        rotate.z -= .05;
    if (keys[SDL_SCANCODE_A])
        rotate.z += .05;

    vector3d_add(self->rotation, self->rotation, rotate);
    TRAINER_ROT_Z = self->rotation.z;

    // z is up
    float yaw = self->rotation.z;

    vector3d_set(forward, -sin(yaw) * 30, cos(yaw) * 30, 0);
    vector3d_set(moveDir, 0, 0, 0);

    if (keys[SDL_SCANCODE_W])
        vector3d_add(moveDir, moveDir, -forward);
    else if (keys[SDL_SCANCODE_S])
        vector3d_add(moveDir, moveDir, forward);
    vector3d_copy(self->previousPosition, self->position);
    vector3d_add(self->position, self->position, moveDir);
    TRAINER_X = self->position.x;
    TRAINER_Y = self->position.y;
}

void trainer_collide(struct Entity_S *self, struct Entity_S *other)
{
    slog("trainer collided with %s", other->name);
    if(other->type == ET_POKEMON)
    {
        slog("trainer collided with pokemon");
        self->position = self->previousPosition;

        //Start battle
    }
    if(other->type == ET_INTERACTABLE)
    {
        slog("trainer collided with interactable");
        self->position = self->previousPosition;

        //Depending on the interactable, do something
    }

}

/*eol@eof*/
