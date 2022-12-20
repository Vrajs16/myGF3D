#include "simple_logger.h"
#include "gfc_primitives.h"
#include "gf3d_draw.h"
#include "trainer.h"
#include "gf2d_windows_common.h"
#include "soundmanager.h"
#include "multiplayer.h"
#include "pokemon.h";

float TRAINER_X = 0;
float TRAINER_Y = 0;
float TRAINER_Z = 1000;
float TRAINER_ROT_Z = 0;

extern int WORLD_BOUND_X1;
extern int WORLD_BOUND_Y1;
extern int WORLD_BOUND_X2;
extern int WORLD_BOUND_Y2;

extern int EVOLVE_ANIMATION;
extern int CAN_EVOLVE;

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

extern Sprite *BATTLE_SPRITE;

extern int MULTIPLAYER;

int MOVING = 0;

int ANIMATION_FRAME_RUNNING = 0;
int ANIMATION_INTERVAL_RUNNING = 0;

int ANIMATION_ROCK_PLAYING = 0;
int ANIMATION_FRAME_ROCK = 0;

int ANIMATION_TREE_PLAYING = 0;
int ANIMATION_FRAME_TREE = 0;
int ANIMAITON_INTERVAL_TREE = 0;

int ANIMATION_FRAME_IDLE = 0;
int ANIMATION_INTERVAL_IDLE = 0;
Vector3D STRENGTH_FINAL_POSITION = {0, 0, 0};
float STRENGTH_FINAL_ROTATION = 0;
int ANIMATION_STRENGTH_PLAYING = 0;
int ANIMATION_STRENGTH_MAX = 50;

int CURRENT_COLLISION_ENTITY_ID = 0;

int startingAnimationEvolution = 1;
float animationFinishRot;
float animationRot;
Entity *animationFinishEntity;

Entity *OtherTrainer;

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

    if (MULTIPLAYER)
    {
        // Make other trainer
        OtherTrainer = entity_new();

        if (!OtherTrainer)
        {
            slog("UGH OHHHH, no trainer for you!");
            return NULL;
        }
        // memcopy over the model
        OtherTrainer->runAniModels = gfc_allocate_array(sizeof(Model), 17);
        OtherTrainer->idleAniModels = gfc_allocate_array(sizeof(Model), 45);

        for (int i = 0; i < 17; i++)
        {
            snprintf(modelfilename, GFCLINELEN, "assets/trainer/%s/running/%s%d.obj", trainer, trainer, i + 1);
            OtherTrainer->runAniModels[i] = gf3d_model_load_full(modelfilename, texturefilename);
        }

        for (int i = 0; i < 45; i++)
        {
            snprintf(modelfilename, GFCLINELEN, "assets/trainer/%s/idle/%s%d.obj", trainer, trainer, i + 1);
            OtherTrainer->idleAniModels[i] = gf3d_model_load_full(modelfilename, texturefilename);
        }

        OtherTrainer->name = trainer;
        OtherTrainer->type = ET_TRAINER;
        OtherTrainer->model = OtherTrainer->idleAniModels[0];
        OtherTrainer->isBox = 1;
        OtherTrainer->boundingBox = gfc_box(0, 0, 350, 200, 200, 350);
        vector3d_copy(OtherTrainer->scale, vector3d(scale, scale, scale));
        vector3d_copy(OtherTrainer->rotation, rotation);
        vector3d_copy(OtherTrainer->previousPosition, position);
        vector3d_copy(OtherTrainer->position, vector3d(0, 0, -1000));
    }
    return ent;
}

void trainer_think(Entity *self)
{
    if (MULTIPLAYER && sending(self->position.x, self->position.y, self->position.z, self->rotation.z, MOVING, ANIMATION_FRAME_RUNNING, ANIMATION_FRAME_IDLE))
        receiving();

    if (!self)
        return;

    if (BATTLE)
        return;
    float evolveAnimationRotation = self->rotation.z;
    if (EVOLVE_ANIMATION)
    {
        if (startingAnimationEvolution)
        {
            animationFinishEntity = pokemon_new_name(vector3d(self->position.x + cos(self->rotation.z - M_PI_2) * 2000, self->position.y + sin(self->rotation.z - M_PI_2) * 2000, 0), vector3d(0, 0, self->rotation.z - M_PI), BATTLE_POKEMON->pokemon.evolutionName);
            animationFinishRot = evolveAnimationRotation + (4 * M_PI);
            BATTLE_POKEMON->position = vector3d(self->position.x + cos(self->rotation.z - M_PI_2) * 2000, self->position.y + sin(self->rotation.z - M_PI_2) * 2000, 0);
            BATTLE_POKEMON->rotation = vector3d(0, 0, self->rotation.z - M_PI);
            TRAINER_X = BATTLE_POKEMON->position.x;
            TRAINER_Y = BATTLE_POKEMON->position.y;
            startingAnimationEvolution = 0;
            animationRot = 0;
        }
        if (TRAINER_ROT_Z < animationFinishRot)
        {

            // Scale the animationFinishEntity and The BATTLE_POKEMON so that they look like they are evolving and switching places, use animationRot
            if (animationRot > 8 * M_PI)
            {
                animationFinishEntity->scale = vector3d(animationFinishEntity->pokemon.scale, animationFinishEntity->pokemon.scale, animationFinishEntity->pokemon.scale);
            }
            else
            {
                animationFinishEntity->scale = vector3d(animationFinishEntity->pokemon.scale * sin(animationRot), animationFinishEntity->pokemon.scale * sin(animationRot), animationFinishEntity->pokemon.scale * sin(animationRot));
                BATTLE_POKEMON->scale = vector3d(BATTLE_POKEMON->pokemon.scale * sin(animationRot + M_PI), BATTLE_POKEMON->pokemon.scale * sin(animationRot + M_PI), BATTLE_POKEMON->pokemon.scale * sin(animationRot + M_PI));
                animationRot += M_PI / 40;
            }
            TRAINER_ROT_Z += M_PI_4 / 40;
            return;
        }
        else
        {
            EVOLVE_ANIMATION = 0;
            CAN_EVOLVE = 0;
            TRAINER_ROT_Z = animationFinishRot;
            self->rotation.z = animationFinishRot;
            TRAINER_X = self->position.x;
            TRAINER_Y = self->position.y;

            // Need to set BATTLE_POKEMON to the new pokemon
            entity_free(BATTLE_POKEMON);
            gf2d_sprite_free(BATTLE_SPRITE);
            BATTLE_POKEMON = animationFinishEntity;
            BATTLE_POKEMON->scale = vector3d(BATTLE_POKEMON->pokemon.scale, BATTLE_POKEMON->pokemon.scale, BATTLE_POKEMON->pokemon.scale);
            BATTLER_HEALTH_MAX = (float)BATTLE_POKEMON->pokemon.health;
            NEW_BATTLER_HEALTH = BATTLER_HEALTH_MAX;
            BATTLER_HEALTH = BATTLER_HEALTH_MAX;
            BATTLE_POKEMON->rotation = vector3d(0, 0, M_PI);
            BATTLE_POKEMON->position = vector3d(0, -2000, -10000);
            TextLine pokemon_sprite;
            snprintf(pokemon_sprite, GFCLINELEN, "assets/content_editor/%s.png", BATTLE_POKEMON->pokemon.name);
            BATTLE_SPRITE = gf2d_sprite_load_image(pokemon_sprite);
        }
        // CAN_EVOLVE = 0;
        // EVOLVE_ANIMATION = 0;
        // After the animation need to update the battle pokemon and all of the stats like health name, etc and put the pokemon back in the previous position
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

    vector3d_set(forward, -sin(yaw) * 50, cos(yaw) * 50, 0);
    vector3d_set(moveDir, 0, 0, 0);

    if (keys[SDL_SCANCODE_W])
    {
        MOVING = 1;

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
        if (ANIMATION_FRAME_RUNNING == 4 || ANIMATION_FRAME_RUNNING == 12)
        {
            playSound("walking", 0, .2, 3, 1);
        }
        if (ANIMATION_FRAME_RUNNING > 16)
            ANIMATION_FRAME_RUNNING = 1;
        ANIMATION_INTERVAL_RUNNING++;
    }
    else if (keys[SDL_SCANCODE_S])
    {
        MOVING = 0;
        vector3d_add(moveDir, moveDir, forward);
    }
    else
    {
        MOVING = 0;
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

    // Check if player is out of bounds
    if (self->position.x < WORLD_BOUND_X1 || self->position.x > WORLD_BOUND_X2 || self->position.y < WORLD_BOUND_Y1 || self->position.y > WORLD_BOUND_Y2)
        vector3d_copy(self->position, self->previousPosition);

    TRAINER_X = self->position.x;
    TRAINER_Y = self->position.y;
    TRAINER_Z = 1000;

    if (SIGN_COLLISION == 1 && vector3d_equal(self->position, self->previousPosition))
        SIGN_COLLISION = 1;
    else
        SIGN_COLLISION = 0;

    if (PC_COLLISION == 1 && vector3d_equal(self->position, self->previousPosition))
    {
        if (keys[SDL_SCANCODE_E])
        {
            slog("You healed your Pokemon!");
            BATTLER_HEALTH_MAX = (float)BATTLE_POKEMON->pokemon.health;
            NEW_BATTLER_HEALTH = BATTLER_HEALTH_MAX;
            BATTLER_HEALTH = BATTLER_HEALTH_MAX;
            sprintf(BATTLER_HEALTH_TEXT, "%d%%", (int)(BATTLER_HEALTH / BATTLER_HEALTH_MAX * 100));
            PC_COLLISION = 0;
            BATTLER_POKEMON_DEAD = 0;
            playSound("healing", 0, 5, 5, 1);
        }
    }
    else
        PC_COLLISION = 0;

    if ((TREE_COLLISION == 1 && vector3d_equal(self->position, self->previousPosition)) || ANIMATION_TREE_PLAYING)
    {
        Entity *tree = entity_get("tree", CURRENT_COLLISION_ENTITY_ID);

        if (keys[SDL_SCANCODE_E] && !ANIMATION_TREE_PLAYING)
        {
            // Delete tree
            ANIMATION_TREE_PLAYING = 1;
            slog("You used Cut!");
            TREE_COLLISION = 0;
            playSound("tree-falling", 0, 5, 7, 1);
        }
        else if (ANIMATION_TREE_PLAYING)
        {
            if (ANIMATION_FRAME_TREE > 99)
            {
                ANIMATION_TREE_PLAYING = 0;
                entity_free(tree);
                TREE_COLLISION = 0;
                ANIMATION_FRAME_TREE = 0;
            }
            else
            {
                tree->model = tree->runAniModels[ANIMATION_FRAME_TREE];
                if (ANIMAITON_INTERVAL_TREE == 2)
                {
                    ANIMATION_FRAME_TREE++;
                    ANIMAITON_INTERVAL_TREE = 0;
                }
                ANIMAITON_INTERVAL_TREE++;
            }
        }
    }
    else
        TREE_COLLISION = 0;

    if ((ROCK_COLLISION == 1 && vector3d_equal(self->position, self->previousPosition)) || ANIMATION_ROCK_PLAYING)
    {
        Entity *rock = entity_get("rock", CURRENT_COLLISION_ENTITY_ID);
        if (keys[SDL_SCANCODE_E] && !ANIMATION_ROCK_PLAYING)
        {
            ANIMATION_ROCK_PLAYING = 1;
            // Delete Rock
            slog("You used Rock Smash!");
            ROCK_COLLISION = 0;
            playSound("rocks-falling", 0, 5, 6, 1);
        }
        else if (ANIMATION_ROCK_PLAYING)
        {
            if (ANIMATION_FRAME_ROCK > 249)
            {
                ANIMATION_ROCK_PLAYING = 0;
                entity_free(rock);
                ROCK_COLLISION = 0;
                ANIMATION_FRAME_ROCK = 0;
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
        Entity *strength = entity_get("strength", CURRENT_COLLISION_ENTITY_ID);
        if (keys[SDL_SCANCODE_E] && !ANIMATION_STRENGTH_PLAYING)
        {
            // Move Rock
            slog("You used Strength!");
            // Move bolder 1500 units in the direction the trainer is facing
            STRENGTH_FINAL_ROTATION = TRAINER_ROT_Z;
            vector3d_set(STRENGTH_FINAL_POSITION, -round(30 * sin(TRAINER_ROT_Z)), -round(30 * cos(TRAINER_ROT_Z + M_PI)), 0);
            STRENGTH_COLLISION = 0;
            ANIMATION_STRENGTH_PLAYING = 1;
        }
        else if (ANIMATION_STRENGTH_PLAYING)
        {
            if (ANIMATION_STRENGTH_MAX > 0)
            {
                slog("Moving rock %d", ANIMATION_STRENGTH_MAX);
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
        other->position.z = -10000;
        other->rotation.z = 0;

        TRAINER_X = 1000;
        TRAINER_Y = -2000;
        TRAINER_Z = -9000;
        TRAINER_ROT_Z = .2 + M_PI;
        OP_POKEMON = other;
        OP_HEALTH_MAX = (float)other->pokemon.health;
        OP_HEALTH = OP_HEALTH_MAX;
        NEW_OP_HEALTH = OP_HEALTH_MAX;
        sprintf(OP_HEALTH_TEXT, "%d%%", (int)(OP_HEALTH / OP_HEALTH_MAX * 100));
        playSound("battle-music", -1, .3, 1, 1);
    }
    if (other->type == ET_INTERACTABLE)
    {
        self->position = self->previousPosition;

        if (ANIMATION_ROCK_PLAYING || ANIMATION_TREE_PLAYING || ANIMATION_STRENGTH_PLAYING)
            return;

        // Depending on the interactable, do something
        if (strcmp(other->name, "sign") == 0 && SIGN_COLLISION == 0)
        {
            SIGN_COLLISION = 1;
            CURRENT_COLLISION_ENTITY_ID = other->entityID;
            return;
        }
        if (strcmp(other->name, "tree") == 0 && TREE_COLLISION == 0)
        {
            TREE_COLLISION = 1;
            CURRENT_COLLISION_ENTITY_ID = other->entityID;
            return;
        }
        if (strcmp(other->name, "rock") == 0 && ROCK_COLLISION == 0)
        {
            ROCK_COLLISION = 1;
            CURRENT_COLLISION_ENTITY_ID = other->entityID;
            return;
        }
        if (strcmp(other->name, "pc") == 0 && PC_COLLISION == 0)
        {
            PC_COLLISION = 1;
            CURRENT_COLLISION_ENTITY_ID = other->entityID;
            return;
        }
        if (strcmp(other->name, "strength") == 0 && STRENGTH_COLLISION == 0)
        {
            STRENGTH_COLLISION = 1;
            CURRENT_COLLISION_ENTITY_ID = other->entityID;
            return;
        }
    }
}

/*eol@eof*/
