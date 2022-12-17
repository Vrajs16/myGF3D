#ifndef __POKEMON_H__
#define __POKEMON_H__

#include "entity.h"

/**
 * @brief Create a new pokemon entity
 * @param position where to spawn the pokemon at
 * @param rotation what direction to face
 * @param pokemon the name of the model to load
 * @param scale how big to make the pokemon
 * @return NULL on error, or an agumon entity pointer on success
 */

typedef struct
{
    float fire;
    float water;
    float grass;
    float physic;
    float dark;
} TypeEffectiveness;

typedef struct
{
    TypeEffectiveness fire;
    TypeEffectiveness water;
    TypeEffectiveness grass;
    TypeEffectiveness physic;
    TypeEffectiveness dark;
} TypeChart;

typedef struct
{
    int total;        // total number of pokemon
    Pokemon *pokemon; // List of pokemon in order
    TypeChart typeChart;
} Pokedex;

Entity *pokemon_new(Vector3D position, Vector3D rotation, Pokemon pokemon, float scale);

void load_pokedex_json(char *filename);

float pokemon_move_multiplier(char *attackMoveType, char *defendType);

Pokedex get_pokedex(void);

Entity *pokemon_new_name(Vector3D position, Vector3D rotation, char *pokemonName);

#endif
