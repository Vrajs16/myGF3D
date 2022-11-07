
#include "simple_logger.h"
#include "pokemon.h"

void pokemon_think(Entity *self);

Entity *pokemon_new(Vector3D position, Vector3D rotation, Pokemon pokemon, float scale)
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
    snprintf(modelfilename, GFCLINELEN, "assets/pokemon/%s/%s.obj", pokemon.name, pokemon.name);
    snprintf(texturefilename, GFCLINELEN, "assets/pokemon/%s/%s-bake.png", pokemon.name, pokemon.name);
    ent->model = gf3d_model_load_full(modelfilename, texturefilename);
    ent->think = pokemon_think;

    ent->isBox = 1;
    ent->boundingBox = pokemon.boundingBox;
    ent->name = pokemon.name;
    ent->type = ET_POKEMON;
    ent->pokemon = pokemon;
    vector3d_copy(ent->scale, vector3d(scale, scale, scale));
    vector3d_copy(ent->position, position);
    vector3d_copy(ent->rotation, rotation);

    return ent;
}

void pokemon_think(Entity *self)
{
    if (!self)
        return;

    // self->rotation.z += -0.005;

    // self->position.x = 1000 * cos(self->rotation.z + M_PI_2);
    // self->position.y = 1000 * sin(self->rotation.z + M_PI_2);
}

Pokedex *load_pokedex_json(char *filename)
{
    Pokedex *pokedex = NULL;
    pokedex = gfc_allocate_array(sizeof(Pokedex), 1);
    // SJson *json, *pokedex_j, *typechart;
    SJson *json, *pokedex_j;
    int totalPokemon;
    json = sj_load(filename);
    if (!json)
    {
        slog("failed to load %s", filename);
        sj_free(json);
        return NULL;
    }
    pokedex_j = sj_object_get_value(json, "pokedex");
    if (!pokedex_j)
    {
        slog("failed to find pokedex in %s", filename);
        sj_free(json);
        return NULL;
    }
    sj_get_integer_value(sj_object_get_value(pokedex_j, "total"), &totalPokemon);
    if (!totalPokemon)
    {
        slog("failed to find total in %s", filename);
        sj_free(json);
        return NULL;
    }
    pokedex->total = totalPokemon;
    pokedex->pokemon = gfc_allocate_array(sizeof(Pokemon), totalPokemon);

    SJson *pokemons = sj_object_get_value(pokedex_j, "pokemons");

    for (int i = 0; i < pokedex->total; i++)
    {
        SJson *pokemon = sj_array_get_nth(pokemons, i);
        if (!pokemon)
        {
            slog("failed to find pokemon %i in %s", i, filename);
            sj_free(json);
            return NULL;
        }
        const char *name = sj_object_get_value_as_string(pokemon, "name");
        if (!name)
        {
            slog("failed to find name in %s", filename);
            sj_free(json);
            return NULL;
        }
        sj_get_float_value(sj_object_get_value(pokemon, "scale"), &pokedex->pokemon[i].scale);
        pokedex->pokemon[i].name = strdup(name);
        SJson *boundingBox = sj_object_get_value(pokemon, "boundingBox");
        if (!boundingBox)
        {
            slog("failed to find boundingBox in %s", filename);
            sj_free(json);
            return NULL;
        }
        sj_get_float_value(sj_object_get_value(boundingBox, "x"), &pokedex->pokemon[i].boundingBox.x);
        sj_get_float_value(sj_object_get_value(boundingBox, "y"), &pokedex->pokemon[i].boundingBox.y);
        sj_get_float_value(sj_object_get_value(boundingBox, "z"), &pokedex->pokemon[i].boundingBox.z);
        sj_get_float_value(sj_object_get_value(boundingBox, "w"), &pokedex->pokemon[i].boundingBox.w);
        sj_get_float_value(sj_object_get_value(boundingBox, "h"), &pokedex->pokemon[i].boundingBox.h);
        sj_get_float_value(sj_object_get_value(boundingBox, "d"), &pokedex->pokemon[i].boundingBox.d);
        const char *type = sj_object_get_value_as_string(pokemon, "type");
        if (!type)
        {
            slog("failed to find type in %s", filename);
            sj_free(json);
            return NULL;
        }
        pokedex->pokemon[i].type = (char * )type;
        sj_get_bool_value(sj_object_get_value(pokemon, "evolution"), &pokedex->pokemon[i].evolution);
        sj_get_integer_value(sj_object_get_value(pokemon, "health"), &pokedex->pokemon[i].health);

        SJson *moves = sj_object_get_value(pokemon, "moves");
        if (!moves)
        {
            slog("failed to find moves in %s", filename);
            sj_free(json);
            return NULL;
        
        }
        for (int j = 0; j < sj_array_get_count(moves); j++)
        {
            SJson *move = sj_array_get_nth(moves, j);
            if (!move)
            {
                slog("failed to find move %i in %s", i, filename);
                sj_free(json);
                return NULL;
            }
            char *move_name = (char *) sj_object_get_value_as_string(move, "move");
            if (!move_name)
            {
                slog("failed to find name in %s", filename);
                sj_free(json);
                return NULL;
            }
            pokedex->pokemon[i].moves[j].move = move_name;
            sj_get_integer_value(sj_object_get_value(move, "power"), &pokedex->pokemon[i].moves[j].power);
            const char *type = sj_object_get_value_as_string(move, "type");
            if (!type)
            {
                slog("failed to find type in %s", filename);
                sj_free(json);
                return NULL;
            }
            pokedex->pokemon[i].moves[j].type = (char * ) type;
        }
    }

    // GOTTA LOAD MY TYPE SYSTEM IN HERE

    // typechart = sj_object_get_value(json, "typechart");
    // if (!typechart)
    // {
    //     slog("failed to find typechart in %s", filename);
    //     sj_free(json);
    //     return NULL;
    // }
    return pokedex;
}

/*eol@eof*/
