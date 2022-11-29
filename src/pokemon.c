
#include "simple_logger.h"
#include "pokemon.h"

static Pokedex POKEDEX;

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

void load_pokedex_json(char *filename)
{
    SJson *json, *pokedex_j, *typechart;
    int totalPokemon;
    json = sj_load(filename);
    if (!json)
    {
        slog("failed to load %s", filename);
        sj_free(json);
        return;
    }
    pokedex_j = sj_object_get_value(json, "pokedex");
    if (!pokedex_j)
    {
        slog("failed to find pokedex in %s", filename);
        sj_free(json);
        return;
    }
    sj_get_integer_value(sj_object_get_value(pokedex_j, "total"), &totalPokemon);
    if (!totalPokemon)
    {
        slog("failed to find total in %s", filename);
        sj_free(json);
        return;
    }
    POKEDEX.total = totalPokemon;
    POKEDEX.pokemon = gfc_allocate_array(sizeof(Pokemon), totalPokemon);

    SJson *pokemons = sj_object_get_value(pokedex_j, "pokemons");

    for (int i = 0; i < POKEDEX.total; i++)
    {
        SJson *pokemon = sj_array_get_nth(pokemons, i);
        if (!pokemon)
        {
            slog("failed to find pokemon %i in %s", i, filename);
            sj_free(json);
            return;
        }
        const char *name = sj_object_get_value_as_string(pokemon, "name");
        if (!name)
        {
            slog("failed to find name in %s", filename);
            sj_free(json);
            return;
        }
        sj_get_float_value(sj_object_get_value(pokemon, "scale"), &POKEDEX.pokemon[i].scale);
        POKEDEX.pokemon[i].name = strdup(name);
        SJson *boundingBox = sj_object_get_value(pokemon, "boundingBox");
        if (!boundingBox)
        {
            slog("failed to find boundingBox in %s", filename);
            sj_free(json);
            return;
        }
        sj_get_float_value(sj_object_get_value(boundingBox, "x"), &POKEDEX.pokemon[i].boundingBox.x);
        sj_get_float_value(sj_object_get_value(boundingBox, "y"), &POKEDEX.pokemon[i].boundingBox.y);
        sj_get_float_value(sj_object_get_value(boundingBox, "z"), &POKEDEX.pokemon[i].boundingBox.z);
        sj_get_float_value(sj_object_get_value(boundingBox, "w"), &POKEDEX.pokemon[i].boundingBox.w);
        sj_get_float_value(sj_object_get_value(boundingBox, "h"), &POKEDEX.pokemon[i].boundingBox.h);
        sj_get_float_value(sj_object_get_value(boundingBox, "d"), &POKEDEX.pokemon[i].boundingBox.d);
        const char *type = sj_object_get_value_as_string(pokemon, "type");
        if (!type)
        {
            slog("failed to find type in %s", filename);
            sj_free(json);
            return;
        }
        POKEDEX.pokemon[i].type = (char *)type;
        sj_get_bool_value(sj_object_get_value(pokemon, "evolution"), &POKEDEX.pokemon[i].evolution);
        sj_get_integer_value(sj_object_get_value(pokemon, "health"), &POKEDEX.pokemon[i].health);

        SJson *moves = sj_object_get_value(pokemon, "moves");
        if (!moves)
        {
            slog("failed to find moves in %s", filename);
            sj_free(json);
            return;
        }
        for (int j = 0; j < sj_array_get_count(moves); j++)
        {
            SJson *move = sj_array_get_nth(moves, j);
            if (!move)
            {
                slog("failed to find move %i in %s", i, filename);
                sj_free(json);
                return;
            }
            char *move_name = (char *)sj_object_get_value_as_string(move, "move");
            if (!move_name)
            {
                slog("failed to find name in %s", filename);
                sj_free(json);
                return;
            }
            POKEDEX.pokemon[i].moves[j].move = move_name;
            sj_get_integer_value(sj_object_get_value(move, "power"), &POKEDEX.pokemon[i].moves[j].power);
            const char *type = sj_object_get_value_as_string(move, "type");
            if (!type)
            {
                slog("failed to find type in %s", filename);
                sj_free(json);
                return;
            }
            POKEDEX.pokemon[i].moves[j].type = (char *)type;
        }
    }

    // GOTTA LOAD MY TYPE SYSTEM IN HERE

    typechart = sj_object_get_value(json, "typechart");
    if (!typechart)
    {
        slog("failed to find typechart in %s", filename);
        sj_free(json);
        return;
    }

    SJson *fire = sj_object_get_value(typechart, "fire");
    SJson *water = sj_object_get_value(typechart, "water");
    SJson *grass = sj_object_get_value(typechart, "grass");
    SJson *physic = sj_object_get_value(typechart, "physic");
    SJson *dark = sj_object_get_value(typechart, "dark");
    if (!fire || !water || !grass || !physic || !dark)
    {
        slog("failed to find typechart in %s", filename);
        sj_free(json);
        return;
    }

    sj_get_float_value(sj_object_get_value(fire, "fire"), &POKEDEX.typeChart.fire.fire);
    sj_get_float_value(sj_object_get_value(fire, "water"), &POKEDEX.typeChart.fire.water);
    sj_get_float_value(sj_object_get_value(fire, "grass"), &POKEDEX.typeChart.fire.grass);
    sj_get_float_value(sj_object_get_value(fire, "physic"), &POKEDEX.typeChart.fire.physic);
    sj_get_float_value(sj_object_get_value(fire, "dark"), &POKEDEX.typeChart.fire.dark);

    sj_get_float_value(sj_object_get_value(water, "fire"), &POKEDEX.typeChart.water.fire);
    sj_get_float_value(sj_object_get_value(water, "water"), &POKEDEX.typeChart.water.water);
    sj_get_float_value(sj_object_get_value(water, "grass"), &POKEDEX.typeChart.water.grass);
    sj_get_float_value(sj_object_get_value(water, "physic"), &POKEDEX.typeChart.water.physic);
    sj_get_float_value(sj_object_get_value(water, "dark"), &POKEDEX.typeChart.water.dark);

    sj_get_float_value(sj_object_get_value(grass, "fire"), &POKEDEX.typeChart.grass.fire);
    sj_get_float_value(sj_object_get_value(grass, "water"), &POKEDEX.typeChart.grass.water);
    sj_get_float_value(sj_object_get_value(grass, "grass"), &POKEDEX.typeChart.grass.grass);
    sj_get_float_value(sj_object_get_value(grass, "physic"), &POKEDEX.typeChart.grass.physic);
    sj_get_float_value(sj_object_get_value(grass, "dark"), &POKEDEX.typeChart.grass.dark);

    sj_get_float_value(sj_object_get_value(physic, "fire"), &POKEDEX.typeChart.physic.fire);
    sj_get_float_value(sj_object_get_value(physic, "water"), &POKEDEX.typeChart.physic.water);
    sj_get_float_value(sj_object_get_value(physic, "grass"), &POKEDEX.typeChart.physic.grass);
    sj_get_float_value(sj_object_get_value(physic, "physic"), &POKEDEX.typeChart.physic.physic);
    sj_get_float_value(sj_object_get_value(physic, "dark"), &POKEDEX.typeChart.physic.dark);

    sj_get_float_value(sj_object_get_value(dark, "fire"), &POKEDEX.typeChart.dark.fire);
    sj_get_float_value(sj_object_get_value(dark, "water"), &POKEDEX.typeChart.dark.water);
    sj_get_float_value(sj_object_get_value(dark, "grass"), &POKEDEX.typeChart.dark.grass);
    sj_get_float_value(sj_object_get_value(dark, "physic"), &POKEDEX.typeChart.dark.physic);
    sj_get_float_value(sj_object_get_value(dark, "dark"), &POKEDEX.typeChart.dark.dark);
}

float pokemon_move_multiplier(char *attackMoveType, char *defendType)
{
    float multiplier = 1;
    if (strcmp(attackMoveType, "fire") == 0)
    {
        if (strcmp(defendType, "fire") == 0)
            multiplier = POKEDEX.typeChart.fire.fire;
        else if (strcmp(defendType, "water") == 0)
            multiplier = POKEDEX.typeChart.fire.water;
        else if (strcmp(defendType, "grass") == 0)
            multiplier = POKEDEX.typeChart.fire.grass;
        else if (strcmp(defendType, "physic") == 0)
            multiplier = POKEDEX.typeChart.fire.physic;
        else if (strcmp(defendType, "dark") == 0)
            multiplier = POKEDEX.typeChart.fire.dark;
    }
    else if (strcmp(attackMoveType, "water") == 0)
    {
        if (strcmp(defendType, "fire") == 0)
            multiplier = POKEDEX.typeChart.water.fire;
        else if (strcmp(defendType, "water") == 0)
            multiplier = POKEDEX.typeChart.water.water;
        else if (strcmp(defendType, "grass") == 0)
            multiplier = POKEDEX.typeChart.water.grass;
        else if (strcmp(defendType, "physic") == 0)
            multiplier = POKEDEX.typeChart.water.physic;
        else if (strcmp(defendType, "dark") == 0)
            multiplier = POKEDEX.typeChart.water.dark;
    }
    else if (strcmp(attackMoveType, "grass") == 0)
    {
        if (strcmp(defendType, "fire") == 0)
            multiplier = POKEDEX.typeChart.grass.fire;
        else if (strcmp(defendType, "water") == 0)
            multiplier = POKEDEX.typeChart.grass.water;
        else if (strcmp(defendType, "grass") == 0)
            multiplier = POKEDEX.typeChart.grass.grass;
        else if (strcmp(defendType, "physic") == 0)
            multiplier = POKEDEX.typeChart.grass.physic;
        else if (strcmp(defendType, "dark") == 0)
            multiplier = POKEDEX.typeChart.grass.dark;
    }
    else if (strcmp(attackMoveType, "physic") == 0)
    {
        if (strcmp(defendType, "fire") == 0)
            multiplier = POKEDEX.typeChart.physic.fire;
        else if (strcmp(defendType, "water") == 0)
            multiplier = POKEDEX.typeChart.physic.water;
        else if (strcmp(defendType, "grass") == 0)
            multiplier = POKEDEX.typeChart.physic.grass;
        else if (strcmp(defendType, "physic") == 0)
            multiplier = POKEDEX.typeChart.physic.physic;
        else if (strcmp(defendType, "dark") == 0)
            multiplier = POKEDEX.typeChart.physic.dark;
    }
    else if (strcmp(attackMoveType, "dark") == 0)
    {
        if (strcmp(defendType, "fire") == 0)
            multiplier = POKEDEX.typeChart.dark.fire;
        else if (strcmp(defendType, "water") == 0)
            multiplier = POKEDEX.typeChart.dark.water;
        else if (strcmp(defendType, "grass") == 0)
            multiplier = POKEDEX.typeChart.dark.grass;
        else if (strcmp(defendType, "physic") == 0)
            multiplier = POKEDEX.typeChart.dark.physic;
        else if (strcmp(defendType, "dark") == 0)
            multiplier = POKEDEX.typeChart.dark.dark;
    }
    return multiplier;
}


Pokedex get_pokedex(void){
    return POKEDEX;
}

/*eol@eof*/
