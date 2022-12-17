#include "simple_logger.h"
#include "interactable.h"
#include "entity.h"
#include "gfc_vector.h"

Entity *interactable_new(Vector3D position, Vector3D rotation, char *filename, float scale)
{
    Entity *ent = NULL;
    TextLine modelfilename;
    TextLine texturefilename;

    ent = entity_new();
    if (!ent)
    {
        slog("UGH OHHHH, no interactable for you!");
        return NULL;
    }
    snprintf(modelfilename, GFCLINELEN, "assets/interactables/%s/%s.obj", filename, filename);
    snprintf(texturefilename, GFCLINELEN, "assets/interactables/%s/%s-bake.png", filename, filename);
    ent->model = gf3d_model_load_full(modelfilename, texturefilename);

    if(strcmp("rock", filename) == 0){
        ent->runAniModels = gfc_allocate_array(sizeof(Model), 250);
        for (int i = 0; i < 250; i++)
        {
            snprintf(modelfilename, GFCLINELEN, "assets/interactables/%s/breaking/%s%d.obj", filename, filename, i + 1);
            ent->runAniModels[i] = gf3d_model_load_full(modelfilename, texturefilename);
        }
    }

    if(strcmp("tree", filename) == 0){
        ent->runAniModels = gfc_allocate_array(sizeof(Model), 100);
        for (int i = 0; i < 100; i++)
        {
            snprintf(modelfilename, GFCLINELEN, "assets/interactables/%s/breaking/%s%d.obj", filename, filename, i + 1);
            ent->runAniModels[i] = gf3d_model_load_full(modelfilename, texturefilename);
        }
    }


    ent->isBox = 1;
    ent->boundingBox = gfc_box(0, 0, 350, 350, 350, 350);
    ent->name = filename;
    ent->type = ET_INTERACTABLE;
    vector3d_copy(ent->scale, vector3d(scale, scale, scale));
    vector3d_copy(ent->rotation, rotation);
    vector3d_copy(ent->position, position);
    return ent;
}
