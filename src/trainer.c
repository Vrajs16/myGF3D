#include "simple_logger.h"
#include "gfc_primitives.h"
#include "gf3d_draw.h"
#include "trainer.h"
#include "gf2d_windows_common.h"

float TRAINER_X = 0;
float TRAINER_Y = 0;
float TRAINER_Z = 1000;
float TRAINER_ROT_Z = 0;

int SIGN_COLLISION = 0;
int TREE_COLLISION = 0;
int PC_COLLISION = 0;
int ROCK_COLLISION = 0;
int STRENGTH_COLLISION = 0;

int BATTLE = 0;
Entity *OP_POKEMON = NULL;
float OP_HEALTH;
float NEW_OP_HEALTH;
float OP_HEALTH_MAX;
char OP_HEALTH_TEXT[5];

extern Entity *BATTLE_POKEMON;
extern float BATTLER_HEALTH;
extern float BATTLER_HEALTH_MAX;
extern float NEW_BATTLER_HEALTH;
extern char BATTLER_HEALTH_TEXT[5];
extern int BATTLER_POKEMON_DEAD;

int ANIMATION_FRAME_RUNNING = 0;
int ANIMATION_INTERVAL_RUNNING = 0;

int ANIMATION_ROCK_PLAYING = 0;
int ANIMATION_FRAME_ROCK = 0;

int ANIMATION_FRAME_IDLE = 0;
int ANIMATION_INTERVAL_IDLE = 0;
Vector3D STRENGTH_FINAL_POSITION = {0, 0, 0};
float STRENGTH_FINAL_ROTATION = 0;
int ANIMATION_STRENGTH_PLAYING = 0;
int ANIMATION_STRENGTH_MAX = 50;
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
    snprintf(texturefilename, GFCLINELEN, "assets/trainer/%s/%s-bake.png", trainer, trainer);

    ent->runAniModels = gfc_allocate_array(sizeof(Model), 17);
    ent->idleAniModels = gfc_allocate_array(sizeof(Model), 45);

    for (int i = 0; i < 17; i++)
    {
        snprintf(modelfilename, GFCLINELEN, "assets/trainer/%s/running/%s%d.obj", trainer, trainer, i + 1);
        ent->runAniModels[i] = gf3d_model_load_full(modelfilename, texturefilename);
    }

    for (int i = 0; i < 45; i++)
    {
        snprintf(modelfilename, GFCLINELEN, "assets/trainer/%s/idle/%s%d.obj", trainer, trainer, i + 1);
        ent->idleAniModels[i] = gf3d_model_load_full(modelfilename, texturefilename);
    }

    ent->model = ent->idleAniModels[0];

    ent->isBox = 1;
    ent->boundingBox = gfc_box(0, 0, 350, 200, 200, 350);

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

    if (BATTLE)
    {
        return;
    }

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
    {

        ANIMATION_FRAME_IDLE = 0;
        ANIMATION_INTERVAL_IDLE = 0;

        vector3d_add(moveDir, moveDir, -forward);
        // need to iterate through the array of models and set the current model to the next one
        self->model = self->runAniModels[ANIMATION_FRAME_RUNNING];

        if (ANIMATION_INTERVAL_RUNNING == 2)
        {
            ANIMATION_FRAME_RUNNING++;
            ANIMATION_INTERVAL_RUNNING = 0;
        }
        if (ANIMATION_FRAME_RUNNING > 16)
            ANIMATION_FRAME_RUNNING = 1;
        ANIMATION_INTERVAL_RUNNING++;
    }
    else if (keys[SDL_SCANCODE_S])
        vector3d_add(moveDir, moveDir, forward);
    else
    {
        ANIMATION_FRAME_RUNNING = 0;
        ANIMATION_INTERVAL_RUNNING = 0;

        self->model = self->idleAniModels[ANIMATION_FRAME_IDLE];

        if (ANIMATION_INTERVAL_IDLE == 3)
        {
            ANIMATION_FRAME_IDLE++;
            ANIMATION_INTERVAL_IDLE = 0;
        }
        if (ANIMATION_FRAME_IDLE > 44)
            ANIMATION_FRAME_IDLE = 1;
        ANIMATION_INTERVAL_IDLE++;
    }
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
            slog("You healed your Pokemon!");
            BATTLER_HEALTH_MAX = (float)BATTLE_POKEMON->pokemon.health;
            NEW_BATTLER_HEALTH = BATTLER_HEALTH_MAX;
            BATTLER_HEALTH = BATTLER_HEALTH_MAX;
            sprintf(BATTLER_HEALTH_TEXT, "%d%%", (int)(BATTLER_HEALTH / BATTLER_HEALTH_MAX * 100));
            PC_COLLISION = 0;
            BATTLER_POKEMON_DEAD = 0;
        }
    }
    else
        PC_COLLISION = 0;

    if ((ROCK_COLLISION == 1 && vector3d_equal(self->position, self->previousPosition)) || ANIMATION_ROCK_PLAYING)
    {
        Entity *rock = entity_get("rock");
        if (keys[SDL_SCANCODE_E] && !ANIMATION_ROCK_PLAYING)
        {
            ANIMATION_ROCK_PLAYING = 1;
            // Delete Rock
            slog("You used Rock Smash!");
            ROCK_COLLISION = 0;
        }
        else if (ANIMATION_ROCK_PLAYING)
        {
            if (ANIMATION_FRAME_ROCK > 249)
            {
                ANIMATION_ROCK_PLAYING = 0;
                entity_free(rock);
            }
            else
            {
                rock->model = rock->runAniModels[ANIMATION_FRAME_ROCK];
                ANIMATION_FRAME_ROCK++;
            }
        }
    }
    else
        ROCK_COLLISION = 0;

    if ((STRENGTH_COLLISION == 1 && vector3d_equal(self->position, self->previousPosition)) || ANIMATION_STRENGTH_PLAYING)
    {
        Entity *strength = entity_get("strength");
        if (keys[SDL_SCANCODE_E] && !ANIMATION_STRENGTH_PLAYING)
        {
            // Move Rock
            slog("You used Strength!");
            // Move bolder 1500 units in the direction the trainer is facing
            STRENGTH_FINAL_ROTATION = TRAINER_ROT_Z;
            vector3d_set(STRENGTH_FINAL_POSITION, round(30 * sin(TRAINER_ROT_Z)), round(30 * cos(TRAINER_ROT_Z + M_PI)), 0);
            STRENGTH_COLLISION = 0;
            ANIMATION_STRENGTH_PLAYING = 1;
        }
        else if (ANIMATION_STRENGTH_PLAYING)
        {
            if (ANIMATION_STRENGTH_MAX > 0)
            {
                vector3d_add(strength->position, strength->position, STRENGTH_FINAL_POSITION);
                ANIMATION_STRENGTH_MAX--;
            }
            else
            {
                ANIMATION_STRENGTH_PLAYING = 0;
                ANIMATION_STRENGTH_MAX = 50;
            }
        }
    }
    else
        STRENGTH_COLLISION = 0;

    TRAINER_X = self->position.x;
    TRAINER_Y = self->position.y;
    TRAINER_Z = 1000;
}

void trainer_collide(struct Entity_S *self, struct Entity_S *other)
{
    if (other->type == ET_POKEMON)
    {
        // slog("trainer collided with %s", other->pokemon.name);
        self->position = self->previousPosition;

        // Start battle - Combat Scene + battling logic
        if (BATTLER_HEALTH <= 0)
        {
            return;
        }
        BATTLE = 1;

        // Move trainer and pokemon to battle box
        other->position.x = 0;
        other->position.y = 2000;
        other->position.z = 5000;

        TRAINER_X = 1000;
        TRAINER_Y = -2000;
        TRAINER_Z = 6000;
        TRAINER_ROT_Z = M_PI + .2;
        OP_POKEMON = other;
        OP_HEALTH_MAX = (float)other->pokemon.health;
        OP_HEALTH = OP_HEALTH_MAX;
        NEW_OP_HEALTH = OP_HEALTH_MAX;
        sprintf(OP_HEALTH_TEXT, "%d%%", (int)(OP_HEALTH / OP_HEALTH_MAX * 100));
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