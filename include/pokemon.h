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
Entity *pokemon_new(Vector3D position, Vector3D rotation, char *pokemon, float scale);

#endif
