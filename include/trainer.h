#ifndef __TRAINER_H__
#define __TRAINER_H__

#include "entity.h"

/**
 * @brief Create a new trainer entity
 * @param position where to spawn the trainer at
 * @param rotation what direction to face
 * @param trainer the name of the model to load
 * @param scale how big to make the trainer
 * @return NULL on error, or an agumon entity pointer on success
 */
Entity *trainer_new(Vector3D position, Vector3D rotation, char *trainer, float scale);

#endif
