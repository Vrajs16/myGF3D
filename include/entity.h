#ifndef __ENTITY_H__
#define __ENTITY_H__

#include "gfc_types.h"
#include "gf3d_model.h"

typedef struct
{
    char *move;
    char *type;
    int power;
    int accuracy;
    int pp;
} Move;
typedef struct
{
    char *name;
    int level;
    float scale;
    short evolution;
    char *evolutionName;
    int health;
    char *type;
    Box boundingBox;
    Move moves[4];
} Pokemon;
enum entityTypes
{
    ET_NONE,
    ET_TRAINER,
    ET_POKEMON,
    ET_INTERACTABLE
};

typedef struct Entity_S
{
    Uint8 _inuse;                                                                    /**<keeps track of memory usage*/
    Matrix4 modelMat;                                                                /**<orientation matrix for the model*/
    Model *model;                                                                    /**<pointer to the entity model to draw  (optional)*/
    void (*think)(struct Entity_S *self);                                            /**<pointer to the think function*/
    void (*update)(struct Entity_S *self);                                           /**<pointer to the update function*/
    void (*draw)(struct Entity_S *self);                                             /**<pointer to an optional extra draw funciton*/
    void (*damage)(struct Entity_S *self, float damage, struct Entity_S *inflictor); /**<pointer to the think function*/
    void (*onDeath)(struct Entity_S *self);                                          /**<pointer to an funciton to call when the entity dies*/
    int entityID;                                                                    /**<unique ID for the entity*/

    Vector3D previousPosition; /**<position of the entity last frame*/
    Vector3D position;
    Vector3D velocity;
    Vector3D acceleration;

    Vector3D scale;
    Vector3D rotation;

    Uint32 health; /**<entity dies when it reaches zero*/
    // WHATEVER ELSE WE MIGHT NEED FOR ENTITIES
    struct Entity_S *target; /**<entity to target for weapons / ai*/

    void *customData; /**<IF an entity needs to keep track of extra data, we can do it here*/
    int isBox;
    Box boundingBox; /**<bounding box for the entity*/
    char *name;
    enum entityTypes type;
    int collided;

    Pokemon pokemon;

    int NpcBattled;

    // animation models
    Model **runAniModels;
    Model **idleAniModels;

    void (*collide)(struct Entity_S *self, struct Entity_S *other); /**<pointer to the collide function*/

} Entity;

/**
 * @brief initializes the entity subsystem
 * @param maxEntities the limit on number of entities that can exist at the same time
 */
void entity_system_init(Uint32 maxEntities);

/**
 * @brief provide a pointer to a new empty entity
 * @return NULL on error or a valid entity pointer otherwise
 */
Entity *entity_new();

Entity *entity_get(char *name, int id);

/**
 * @brief free a previously created entity from memory
 * @param self the entity in question
 */
void entity_free(Entity *self);

/**
 * @brief Draw an entity in the current frame
 * @param self the entity in question
 */
void entity_draw(Entity *self);

/**
 * @brief draw ALL active entities
 */
void entity_draw_all();

/**
 * @brief Call an entity's think function if it exists
 * @param self the entity in question
 */
void entity_think(Entity *self);

/**
 * @brief run the think functions for ALL active entities
 */
void entity_think_all();

/**
 * @brief run the update functions for ALL active entities
 */
void entity_update_all();

/**
 * @brief Check if entity is colliding
 * @param ent the entity in question
 */
void entity_collide_check(Entity *ent);

/**
 * @brief run the collide functions for ALL active entities
 */
void entity_collide_check_all();

#endif
