
#include "simple_logger.h"
#include "pokemon.h"

void pokemon_think(Entity *self);

Entity *pokemon_new(Vector3D position, Vector3D rotation, char *pokemon, float scale)
{
    Entity *ent = NULL;
    TextLine modelfilename;
    TextLine texturefilename;

    ent = entity_new();
    if (!ent)
    {
        slog("UGH OHHHH, no agumon for you!");
        return NULL;
    }
    snprintf(modelfilename, GFCLINELEN, "assets/pokemon/%s/%s.obj", pokemon, pokemon);
    snprintf(texturefilename, GFCLINELEN, "assets/pokemon/%s/%s.png", pokemon, pokemon);
    ent->model = gf3d_model_load_full(modelfilename, texturefilename);
    ent->think = pokemon_think;

    vector3d_copy(ent->scale, vector3d(scale, scale, scale));
    vector3d_copy(ent->rotation, rotation);
    vector3d_copy(ent->position, position);
    return ent;
}

void pokemon_think(Entity *self)
{
    if (!self)
        return;

    self->rotation.z += -0.001;

    self->position.x = 1000 * cos(self->rotation.z + M_PI_2);
    self->position.y = 1000 * sin(self->rotation.z + M_PI_2);
}

/*eol@eof*/
