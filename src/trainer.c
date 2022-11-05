#include "simple_logger.h"
#include "gfc_primitives.h"
#include "gf3d_draw.h"
#include "trainer.h"

float TRAINER_X = 0;
float TRAINER_Y = 0;
float TRAINER_ROT_Z = 0;

int SIGN_COLLISION = 0;
int TREE_COLLISION = 0;
int PC_COLLISION = 0;
int ROCK_COLLISION = 0;
int STRENGTH_COLLISION = 0;

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
    ent->boundingBox = gfc_box(0, 0, 350, 200, 200, 350);

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
    if (SIGN_COLLISION == 1 && vector3d_equal(self->position, self->previousPosition))
        SIGN_COLLISION = 1;
    else
        SIGN_COLLISION = 0;

    if (TREE_COLLISION == 1 && vector3d_equal(self->position, self->previousPosition))
    {
        TREE_COLLISION = 1;
        if (keys[SDL_SCANCODE_E])
        {
            // Delete tree
            slog("You used Cut!");
            entity_free(entity_get("tree"));
            TREE_COLLISION = 0;
        }
    }
    else
        TREE_COLLISION = 0;

    if (PC_COLLISION == 1 && vector3d_equal(self->position, self->previousPosition))
    {
        PC_COLLISION = 1;
        if (keys[SDL_SCANCODE_E])
        {
            // Delete PC
            slog("You healed your Pokemon!");
            PC_COLLISION = 0;
        }
    }
    else
        PC_COLLISION = 0;

    if (ROCK_COLLISION == 1 && vector3d_equal(self->position, self->previousPosition))
    {
        ROCK_COLLISION = 1;
        if (keys[SDL_SCANCODE_E])
        {
            // Delete Rock
            slog("You used Rock Smash!");
            entity_free(entity_get("rock"));
            ROCK_COLLISION = 0;
        }
    }
    else
        ROCK_COLLISION = 0;

    if (STRENGTH_COLLISION == 1 && vector3d_equal(self->position, self->previousPosition))
    {
        STRENGTH_COLLISION = 1;
        if (keys[SDL_SCANCODE_E])
        {
            // Move Rock
            slog("You used Strength!");
            Entity *strength = entity_get("strength");
            slog("Strength Position: %f, %f", strength->position.x, strength->position.y);
            slog("sin: %f, cos: %f", 1500 * sin(TRAINER_ROT_Z), 1000 * cos(TRAINER_ROT_Z));
            slog("degrees: %f", TRAINER_ROT_Z * 180 / M_PI);

            // Move bolder 100 units in the direction the trainer is facing
            strength->position.x += 1500 * sin(TRAINER_ROT_Z);
            strength->position.y += 1500 * cos(TRAINER_ROT_Z + M_PI);

            STRENGTH_COLLISION = 0;
        }
    }
    else
        STRENGTH_COLLISION = 0;

    TRAINER_X = self->position.x;
    TRAINER_Y = self->position.y;
}

void trainer_collide(struct Entity_S *self, struct Entity_S *other)
{
    if (other->type == ET_POKEMON)
    {
        slog("trainer collided with %s", other->pokemon.name);
        self->position = self->previousPosition;
        // Start battle - Combat Scene + battling logic
    }
    if (other->type == ET_INTERACTABLE)
    {
        self->position = self->previousPosition;

        // Depending on the interactable, do something
        if (strcmp(other->name, "sign") == 0 && SIGN_COLLISION == 0)
        {
            SIGN_COLLISION = 1;
            return;
        }
        else if (strcmp(other->name, "tree") == 0 && TREE_COLLISION == 0)
        {
            TREE_COLLISION = 1;
            return;
        }
        else if (strcmp(other->name, "rock") == 0 && ROCK_COLLISION == 0)
        {
            ROCK_COLLISION = 1;
            return;
        }
        else if (strcmp(other->name, "pc") == 0 && PC_COLLISION == 0)
        {
            PC_COLLISION = 1;
            return;
        }
        else if (strcmp(other->name, "strength") == 0 && STRENGTH_COLLISION == 0)
        {
            STRENGTH_COLLISION = 1;
            return;
        }
    }
}

/*eol@eof*/
