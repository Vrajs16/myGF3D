#ifndef __INTERACTABLE_H__
#define __INTERACTABLE_H__

#include "entity.h"

/**
 * @brief Create a new interactable entity
 * @param position where to spawn the aguman at
 * @return NULL on error, or an player entity pointer on success
 */
Entity *interactable_new(Vector3D position, Vector3D rotation, char *filename);


#endif
