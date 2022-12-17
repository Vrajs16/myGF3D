#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "simple_logger.h"
#include "simple_json.h"
#include "gf3d_vgraphics.h"
#include "gf2d_font.h"
#include "gf2d_draw.h"
#include "content_editor.h"

extern int _done;

const int GRID_CELL_SIZE = 50;
const int GRID_WIDTH = 29;
const int GRID_HEIGHT = 29;

// Scale for saving the grid
const int GRID_SIZE = 100;
const int GRID_SCALE = 20;
const int GRID_SPACING = GRID_SIZE * GRID_SCALE;

const int GRID_SCALE_FACTOR = GRID_SPACING / GRID_CELL_SIZE;

const SDL_Color grid_background = {22, 22, 22, 255}; // Barely Black
const SDL_Color grid_line_color = {44, 44, 44, 255}; // Dark grey
const SDL_Color grid_cursor_ghost_color = {44, 44, 44, 255};
const SDL_Color grid_cursor_color = {255, 255, 255, 255}; // White

const SDL_Rect MENU_BACKGROUND = {
    .x = 0,
    .y = 0,
    .w = GRID_CELL_SIZE * 5,
    .h = GRID_CELL_SIZE * 5,
};

int mouseX;
int mouseY;

int foffsetX;
int foffsetY;

int fWorldX = 0;
int fWorldY = 0;

int fStartPanX = 0;
int fStartPanY = 0;

SDL_bool quit = SDL_FALSE;
SDL_bool mouse_active = SDL_FALSE;
SDL_bool mouse_hover = SDL_FALSE;
SDL_bool pan = SDL_FALSE;

typedef enum
{
    MODEL_MAP,
    MODEL_POKEMON,
    MODEL_INTERACTABLE,
    MODEL_NONE,
} ModelType;

typedef enum
{
    MAP,
    POKEMON,
    INTERACTABLE,
} TextureType;

typedef struct
{
    SDL_Rect rect;
    SDL_Texture *BottomTexture;
    int BottomTextureSet;
    char BottomTextureName[20];
    SDL_Texture *TopTexture;
    int TopTextureSet;
    char TopTextureName[20];
    double TopTextureRotation;
    ModelType BottomTextureType;
    ModelType TopTextureType;
} Grid;

typedef struct
{
    int id;
    char name[20];
    SDL_Rect rect;
    SDL_Texture *texture;
    TextureType type;

} TileTexture;

Grid GRID[GRID_WIDTH][GRID_HEIGHT];

const int MAP_TEXTURE_COUNT = 2;
const int POKEMON_TEXTURE_COUNT = 10;
const int INTERACTABLE_TEXTURE_COUNT = 5;
const int ACTION_TEXTURE_COUNT = 4;

const char *MAP_TEXTURE_NAMES[MAP_TEXTURE_COUNT] = {
    "path",
    "grass",
};
const char *POKEMON_TEXTURE_NAMES[POKEMON_TEXTURE_COUNT] = {
    "growlithe",
    "arcanine",
    "kirlia",
    "gallade",
    "krabby",
    "kingler",
    "skiddo",
    "gogoat",
    "zorua",
    "zoroark",
};

const char *INTERACTABLE_TEXTURE_NAMES[INTERACTABLE_TEXTURE_COUNT] = {
    "strength",
    "pc",
    "rock",
    "tree",
    "sign",
};

const char *ACTION_TEXTURE_NAMES[ACTION_TEXTURE_COUNT] = {
    "save",
    "clean",
    "delete",
    "rotate",
};

TileTexture MAP_TEXTURES[MAP_TEXTURE_COUNT];
TileTexture POKEMON_TEXTURES[POKEMON_TEXTURE_COUNT];
TileTexture INTERACTABLE_TEXTURES[INTERACTABLE_TEXTURE_COUNT];
TileTexture ACTION_TEXTURES[ACTION_TEXTURE_COUNT];
TileTexture CURRENTLY_SELECTED_TEXTURE;

SDL_Rect CURRENTLY_SELECTED_TEXTURE_ITEM;

SDL_Renderer *renderer;

void world_to_screen(int fWorldX, int fWorldY, int *nScreenX, int *nScreenY);
void screen_to_world(int nScreenX, int nScreenY, int *fWorldX, int *fWorldY);
void save_grid(void);
void clean_grid(void);

void content_editor_setup_renderer()
{
    renderer = gf3d_vgraphics_get_renderer();
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_ShowCursor(SDL_ENABLE);

    foffsetX = -gf3d_vgraphics_get_width() / 2 + GRID_CELL_SIZE * GRID_WIDTH / 2;
    foffsetY = -gf3d_vgraphics_get_height() / 2 + GRID_CELL_SIZE * GRID_HEIGHT / 2;

    // Load the tile textures
    for (int i = 0; i < MAP_TEXTURE_COUNT; i++)
    {
        char path[100];
        strcpy(path, "assets/content_editor/");
        strcat(path, MAP_TEXTURE_NAMES[i]);
        strcat(path, ".png");
        MAP_TEXTURES[i].texture = IMG_LoadTexture(renderer, path);
        if (MAP_TEXTURES[i].texture == NULL)
            printf("Error: %s", IMG_GetError());
        MAP_TEXTURES[i].id = i;
        strcpy(MAP_TEXTURES[i].name, MAP_TEXTURE_NAMES[i]);
        MAP_TEXTURES[i].rect.x = 0;
        MAP_TEXTURES[i].rect.y = i * GRID_CELL_SIZE;
        MAP_TEXTURES[i].rect.w = GRID_CELL_SIZE;
        MAP_TEXTURES[i].rect.h = GRID_CELL_SIZE;
        MAP_TEXTURES[i].type = MAP;
    }

    // Load the object textures
    for (int i = 0; i < POKEMON_TEXTURE_COUNT; i++)
    {
        char path[100];
        strcpy(path, "assets/content_editor/");
        strcat(path, POKEMON_TEXTURE_NAMES[i]);
        strcat(path, ".png");
        POKEMON_TEXTURES[i].texture = IMG_LoadTexture(renderer, path);
        if (POKEMON_TEXTURES[i].texture == NULL)
            printf("Error: %s", IMG_GetError());
        POKEMON_TEXTURES[i].id = i;
        strcpy(POKEMON_TEXTURES[i].name, POKEMON_TEXTURE_NAMES[i]);

        // Rectangle should be 2 per row
        POKEMON_TEXTURES[i].rect.x = i % 2 * GRID_CELL_SIZE + GRID_CELL_SIZE;
        POKEMON_TEXTURES[i].rect.y = i / 2 * GRID_CELL_SIZE;
        POKEMON_TEXTURES[i].rect.w = GRID_CELL_SIZE;
        POKEMON_TEXTURES[i].rect.h = GRID_CELL_SIZE;
        POKEMON_TEXTURES[i].type = POKEMON;
    }

    // Load Intertables
    for (int i = 0; i < INTERACTABLE_TEXTURE_COUNT; i++)
    {
        char path[100];
        strcpy(path, "assets/content_editor/");
        strcat(path, INTERACTABLE_TEXTURE_NAMES[i]);
        strcat(path, ".png");
        INTERACTABLE_TEXTURES[i].texture = IMG_LoadTexture(renderer, path);
        if (INTERACTABLE_TEXTURES[i].texture == NULL)
            printf("Error: %s", IMG_GetError());
        INTERACTABLE_TEXTURES[i].id = i;
        strcpy(INTERACTABLE_TEXTURES[i].name, INTERACTABLE_TEXTURE_NAMES[i]);

        INTERACTABLE_TEXTURES[i].rect.x = GRID_CELL_SIZE * 3;
        INTERACTABLE_TEXTURES[i].rect.y = i * GRID_CELL_SIZE;
        INTERACTABLE_TEXTURES[i].rect.w = GRID_CELL_SIZE;
        INTERACTABLE_TEXTURES[i].rect.h = GRID_CELL_SIZE;
        INTERACTABLE_TEXTURES[i].type = INTERACTABLE;
    }

    // Load Actions
    for (int i = 0; i < ACTION_TEXTURE_COUNT; i++)
    {
        char path[100];
        strcpy(path, "assets/content_editor/");
        strcat(path, ACTION_TEXTURE_NAMES[i]);
        strcat(path, ".png");
        ACTION_TEXTURES[i].texture = IMG_LoadTexture(renderer, path);
        if (ACTION_TEXTURES[i].texture == NULL)
            printf("Error: %s", IMG_GetError());
        ACTION_TEXTURES[i].id = i;
        strcpy(ACTION_TEXTURES[i].name, ACTION_TEXTURE_NAMES[i]);

        ACTION_TEXTURES[i].rect.x = GRID_CELL_SIZE * 4;
        ACTION_TEXTURES[i].rect.y = i * GRID_CELL_SIZE;
        ACTION_TEXTURES[i].rect.w = GRID_CELL_SIZE;
        ACTION_TEXTURES[i].rect.h = GRID_CELL_SIZE;
    }

    // Set the GRID
    for (int x = 0; x < GRID_WIDTH; x++)
    {
        for (int y = 0; y < GRID_HEIGHT; y++)
        {
            GRID[x][y].rect.x = x * GRID_CELL_SIZE;
            GRID[x][y].rect.y = y * GRID_CELL_SIZE;
            GRID[x][y].rect.w = GRID_CELL_SIZE;
            GRID[x][y].rect.h = GRID_CELL_SIZE;
            if (x == GRID_WIDTH / 2 && y == GRID_HEIGHT / 2)
            {
                GRID[x][y].BottomTexture = MAP_TEXTURES[1].texture;
                strcpy(GRID[x][y].BottomTextureName, MAP_TEXTURE_NAMES[1]);
            }
            else
            {
                GRID[x][y].BottomTexture = MAP_TEXTURES[0].texture;
                strcpy(GRID[x][y].BottomTextureName, MAP_TEXTURE_NAMES[0]);
            }
            GRID[x][y].BottomTextureSet = 1;
            GRID[x][y].BottomTextureType = MODEL_MAP;
            GRID[x][y].TopTexture = NULL;
            GRID[x][y].TopTextureSet = 0;
            strcpy(GRID[x][y].TopTextureName, "");
            GRID[x][y].TopTextureRotation = 0;
            GRID[x][y].TopTextureType = MODEL_NONE;
        }
    }
    CURRENTLY_SELECTED_TEXTURE = MAP_TEXTURES[0];
    CURRENTLY_SELECTED_TEXTURE_ITEM = MAP_TEXTURES[0].rect;
}

void content_editor_update()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            // check if c is pressed
        case SDL_KEYDOWN:
            if (event.key.keysym.sym == SDLK_c)
            { // center the grid
                foffsetX = -gf3d_vgraphics_get_width() / 2 + GRID_CELL_SIZE * GRID_WIDTH / 2;
                foffsetY = -gf3d_vgraphics_get_height() / 2 + GRID_CELL_SIZE * GRID_HEIGHT / 2;
            }
            break;
        case SDL_MOUSEMOTION:
            SDL_GetMouseState(&mouseX, &mouseY);

            screen_to_world(mouseX, mouseY, &fWorldX, &fWorldY);

            if (pan)
            {
                foffsetX -= mouseX - fStartPanX;
                foffsetY -= mouseY - fStartPanY;
                fStartPanX = mouseX;
                fStartPanY = mouseY;
            }

            if (!mouse_active)
                mouse_active = SDL_TRUE;
            break;
        case SDL_MOUSEBUTTONDOWN:
            if (event.button.button == SDL_BUTTON_LEFT)
            {
                screen_to_world(mouseX, mouseY, &fWorldX, &fWorldY);

                // Check if mouse in action bar using SDL_PointInRect
                SDL_Point point = {mouseX, mouseY};
                for (int i = 0; i < ACTION_TEXTURE_COUNT; i++)
                {
                    if (SDL_PointInRect(&point, &ACTION_TEXTURES[i].rect))
                    {
                        // compare action name to action name in action list
                        if (strcmp(ACTION_TEXTURES[i].name, "save") == 0)
                        {
                            save_grid();
                            CURRENTLY_SELECTED_TEXTURE_ITEM = ACTION_TEXTURES[i].rect;
                            CURRENTLY_SELECTED_TEXTURE.id = -1;
                        }
                        else if (strcmp(ACTION_TEXTURES[i].name, "clean") == 0)
                        {
                            clean_grid();
                            CURRENTLY_SELECTED_TEXTURE_ITEM = ACTION_TEXTURES[i].rect;
                            CURRENTLY_SELECTED_TEXTURE.id = -1;
                        }
                        else if (strcmp(ACTION_TEXTURES[i].name, "delete") == 0)
                        {
                            CURRENTLY_SELECTED_TEXTURE_ITEM = ACTION_TEXTURES[i].rect;
                            CURRENTLY_SELECTED_TEXTURE.id = -2;
                        }
                        else if (strcmp(ACTION_TEXTURES[i].name, "rotate") == 0)
                        {
                            CURRENTLY_SELECTED_TEXTURE_ITEM = ACTION_TEXTURES[i].rect;
                            CURRENTLY_SELECTED_TEXTURE.id = -3;
                        }

                        return;
                    }
                }

                // Check if mouse in map bar using SDL_PointInRect
                for (int i = 0; i < MAP_TEXTURE_COUNT; i++)
                {
                    if (SDL_PointInRect(&point, &MAP_TEXTURES[i].rect))
                    {
                        CURRENTLY_SELECTED_TEXTURE = MAP_TEXTURES[i];
                        CURRENTLY_SELECTED_TEXTURE_ITEM = MAP_TEXTURES[i].rect;
                        return;
                    }
                }

                // Check if mouse in pokemon bar using SDL_PointInRect
                for (int i = 0; i < POKEMON_TEXTURE_COUNT; i++)
                {
                    if (SDL_PointInRect(&point, &POKEMON_TEXTURES[i].rect))
                    {
                        CURRENTLY_SELECTED_TEXTURE = POKEMON_TEXTURES[i];
                        CURRENTLY_SELECTED_TEXTURE_ITEM = POKEMON_TEXTURES[i].rect;
                        return;
                    }
                }

                // Check if mouse in interable bar using SDL_PointInRect
                for (int i = 0; i < INTERACTABLE_TEXTURE_COUNT; i++)
                {
                    if (SDL_PointInRect(&point, &INTERACTABLE_TEXTURES[i].rect))
                    {
                        CURRENTLY_SELECTED_TEXTURE = INTERACTABLE_TEXTURES[i];
                        CURRENTLY_SELECTED_TEXTURE_ITEM = INTERACTABLE_TEXTURES[i].rect;
                        return;
                    }
                }

                // Get the grid position of the mouse
                int gridX = fWorldX / GRID_CELL_SIZE;
                int gridY = fWorldY / GRID_CELL_SIZE;

                // Check if the grid position is valid
                if (gridX >= 0 && gridX < GRID_WIDTH && gridY >= 0 && gridY < GRID_HEIGHT)
                {
                    if (CURRENTLY_SELECTED_TEXTURE.id == -1)
                        return;
                    else if (CURRENTLY_SELECTED_TEXTURE.id == -2)
                    {
                        GRID[gridX][gridY].BottomTexture = NULL;
                        GRID[gridX][gridY].TopTexture = NULL;
                        GRID[gridX][gridY].BottomTextureSet = 0;
                        GRID[gridX][gridY].TopTextureSet = 0;
                        GRID[gridX][gridY].TopTextureRotation = 0;
                        strcpy(GRID[gridX][gridY].TopTextureName, "");
                        strcpy(GRID[gridX][gridY].BottomTextureName, "");
                        GRID[gridX][gridY].BottomTextureType = MODEL_NONE;
                        GRID[gridX][gridY].TopTextureType = MODEL_NONE;
                    }
                    else if (CURRENTLY_SELECTED_TEXTURE.id == -3)
                    {
                        GRID[gridX][gridY].TopTextureRotation += 45;
                        if (GRID[gridX][gridY].TopTextureRotation >= 360)
                            GRID[gridX][gridY].TopTextureRotation = 0;
                    }
                    else if (CURRENTLY_SELECTED_TEXTURE.type == MAP)
                    {
                        GRID[gridX][gridY].BottomTexture = CURRENTLY_SELECTED_TEXTURE.texture;
                        GRID[gridX][gridY].BottomTextureSet = 1;
                        strcpy(GRID[gridX][gridY].BottomTextureName, CURRENTLY_SELECTED_TEXTURE.name);
                        GRID[gridX][gridY].BottomTextureType = MODEL_MAP;
                    }
                    else if (CURRENTLY_SELECTED_TEXTURE.type == MODEL_POKEMON)
                    {
                        GRID[gridX][gridY].TopTexture = CURRENTLY_SELECTED_TEXTURE.texture;
                        GRID[gridX][gridY].TopTextureSet = 1;
                        strcpy(GRID[gridX][gridY].TopTextureName, CURRENTLY_SELECTED_TEXTURE.name);
                        GRID[gridX][gridY].TopTextureType = MODEL_POKEMON;
                    }
                    else if (CURRENTLY_SELECTED_TEXTURE.type == INTERACTABLE)
                    {
                        GRID[gridX][gridY].TopTexture = CURRENTLY_SELECTED_TEXTURE.texture;
                        GRID[gridX][gridY].TopTextureSet = 1;
                        strcpy(GRID[gridX][gridY].TopTextureName, CURRENTLY_SELECTED_TEXTURE.name);
                        GRID[gridX][gridY].TopTextureType = MODEL_INTERACTABLE;
                    }
                }
            }
            if (event.button.button == SDL_BUTTON_RIGHT)
            {
                fStartPanX = mouseX;
                fStartPanY = mouseY;
                pan = SDL_TRUE;
            }
            break;
        case SDL_MOUSEBUTTONUP:
            if (event.button.button == SDL_BUTTON_RIGHT)
            {
                pan = SDL_FALSE;
            }
            break;

        case SDL_WINDOWEVENT:
            if (event.window.event == SDL_WINDOWEVENT_ENTER && !mouse_hover)
                mouse_hover = SDL_TRUE;
            else if (event.window.event == SDL_WINDOWEVENT_LEAVE && mouse_hover)
                mouse_hover = SDL_FALSE;
            break;
        case SDL_QUIT:
            content_editor_cleanup();
            _done = SDL_TRUE;
            break;
        }
    }
}

void content_editor_draw()
{
    // Draw grid background.
    SDL_SetRenderDrawColor(renderer, grid_background.r, grid_background.g,
                           grid_background.b, grid_background.a);
    SDL_RenderClear(renderer);

    for (int x = 0; x < GRID_WIDTH; x++)
    {
        for (int y = 0; y < GRID_HEIGHT; y++)
        {
            int pixel_x, pixel_y;
            world_to_screen(GRID[x][y].rect.x, GRID[x][y].rect.y, &pixel_x,
                            &pixel_y);
            SDL_Rect pixel_rect = {pixel_x, pixel_y, GRID[x][y].rect.w,
                                   GRID[x][y].rect.h};

            if (GRID[x][y].BottomTextureSet)
                SDL_RenderCopy(renderer, GRID[x][y].BottomTexture, NULL, &pixel_rect);
            if (GRID[x][y].TopTextureSet)
                SDL_RenderCopyEx(renderer, GRID[x][y].TopTexture, NULL, &pixel_rect, GRID[x][y].TopTextureRotation, NULL, SDL_FLIP_NONE);

            SDL_SetRenderDrawColor(renderer, grid_line_color.r, grid_line_color.g, grid_line_color.b, grid_line_color.a);
            SDL_RenderDrawRect(renderer, &pixel_rect);
        }
    }

    // Draw hover
    if (mouse_active && mouse_hover)
    {
        // Get the grid position of the mouse
        int gridX = fWorldX / GRID_CELL_SIZE;
        int gridY = fWorldY / GRID_CELL_SIZE;

        // Check if the grid position is valid
        if (gridX >= 0 && gridX < GRID_WIDTH && gridY >= 0 && gridY < GRID_HEIGHT &&
            !GRID[gridX][gridY].BottomTextureSet && !GRID[gridX][gridY].TopTextureSet)
        {
            int pixel_x, pixel_y;
            world_to_screen(GRID[gridX][gridY].rect.x, GRID[gridX][gridY].rect.y, &pixel_x,
                            &pixel_y);
            SDL_Rect pixel_rect = {pixel_x, pixel_y, GRID[gridX][gridY].rect.w,
                                   GRID[gridX][gridY].rect.h};

            SDL_SetRenderDrawColor(renderer, grid_cursor_ghost_color.r, grid_cursor_ghost_color.g,
                                   grid_cursor_ghost_color.b, grid_cursor_ghost_color.a);
            SDL_RenderFillRect(renderer, &pixel_rect);
        }
    }

    // Draw Textures on the side of the screen, you can use the SCREEN_WIDTH and SCREEN_HEIGHT only here

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 40);
    SDL_RenderFillRect(renderer, &MENU_BACKGROUND);

    for (int i = 0; i < MAP_TEXTURE_COUNT; i++)
        SDL_RenderCopy(renderer, MAP_TEXTURES[i].texture, NULL, &MAP_TEXTURES[i].rect);
    for (int i = 0; i < POKEMON_TEXTURE_COUNT; i++)
        SDL_RenderCopy(renderer, POKEMON_TEXTURES[i].texture, NULL, &POKEMON_TEXTURES[i].rect);
    for (int i = 0; i < INTERACTABLE_TEXTURE_COUNT; i++)
        SDL_RenderCopy(renderer, INTERACTABLE_TEXTURES[i].texture, NULL, &INTERACTABLE_TEXTURES[i].rect);
    for (int i = 0; i < ACTION_TEXTURE_COUNT; i++)
        SDL_RenderCopy(renderer, ACTION_TEXTURES[i].texture, NULL, &ACTION_TEXTURES[i].rect);

    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 125);
    SDL_RenderFillRect(renderer, &CURRENTLY_SELECTED_TEXTURE_ITEM);
    SDL_RenderPresent(renderer);
}

void content_editor_cleanup(void)
{
    SDL_DestroyRenderer(renderer);
}

void world_to_screen(int fWorldX, int fWorldY, int *nScreenX, int *nScreenY)
{
    *nScreenX = fWorldX - foffsetX;
    *nScreenY = fWorldY - foffsetY;
}

void screen_to_world(int nScreenX, int nScreenY, int *fWorldX, int *fWorldY)
{
    *fWorldX = nScreenX + foffsetX;
    *fWorldY = nScreenY + foffsetY;
}

void save_grid()
{
    SDL_Log("Saving grid to file...");
    // Need to save the grid into a json file
    /*
        typedef struct
        {
            SDL_Rect rect;
            SDL_Texture *BottomTexture;
            int BottomTextureSet;
            char BottomTextureName[20];
            SDL_Texture *TopTexture;
            int TopTextureSet;
            char TopTextureName[20];
            double TopTextureRotation;
            ModelType BottomTextureType;
            ModelType TopTextureType;
        } Grid;

        Use GRID_SPACING for the location of the grid centered at 0,0



    */

    /*
    Format:
        {
            "scale": GRID_SCALE,
            "world":[
                {
                    "location": [x, y, 0],
                    "bottom_model": "texture_name",
                    "top_model": "texture_name",
                    "top_model_rotation": 0
                },
                ...
            ]
        }
     */

    SJson *json_file = sj_object_new();
    sj_object_insert(json_file, "scale", sj_new_int(GRID_SCALE));
    sj_object_insert(json_file, "spacing", sj_new_int(GRID_SPACING));
    sj_object_insert(json_file, "grid_width", sj_new_int(GRID_WIDTH));
    sj_object_insert(json_file, "grid_height", sj_new_int(GRID_HEIGHT));
    SJson *world = sj_array_new();

    for (int x = 0; x < GRID_WIDTH; x++)
    {
        for (int y = 0; y < GRID_HEIGHT; y++)
        {
            SJson *world_object = sj_object_new();
            SJson *location = sj_array_new();
            sj_array_append(location, sj_new_int(GRID[x][y].rect.x * GRID_SCALE_FACTOR - (GRID_SPACING * GRID_WIDTH / 2 - GRID_SPACING / 2)));
            sj_array_append(location, sj_new_int(GRID[x][y].rect.y * GRID_SCALE_FACTOR - (GRID_SPACING * GRID_WIDTH / 2 - GRID_SPACING / 2)));
            sj_array_append(location, sj_new_int(0));
            sj_object_insert(world_object, "location", location);
            if (GRID[x][y].BottomTextureSet)
                sj_object_insert(world_object, "bottom_model", sj_new_str(GRID[x][y].BottomTextureName));
            else
                sj_object_insert(world_object, "bottom_model", sj_new_str("path"));
            if (GRID[x][y].TopTextureSet)
            {
                sj_object_insert(world_object, "top_model", sj_new_str(GRID[x][y].TopTextureName));
                sj_object_insert(world_object, "top_model_rotation", sj_new_float(GRID[x][y].TopTextureRotation));
                sj_object_insert(world_object, "top_model_type", sj_new_int(GRID[x][y].TopTextureType));
            }
            sj_array_append(world, world_object);
        }
    }

    sj_object_insert(json_file, "world", world);
    sj_save(json_file, "config/generated_world.json");
    SDL_Log("Saved grid to file!");
}

void clean_grid()
{
    for (int x = 0; x < GRID_WIDTH; x++)
    {
        for (int y = 0; y < GRID_HEIGHT; y++)
        {
            GRID[x][y].BottomTexture = NULL;
            GRID[x][y].TopTexture = NULL;
            GRID[x][y].BottomTextureSet = 0;
            GRID[x][y].TopTextureSet = 0;
            GRID[x][y].TopTextureRotation = 0;
            strcpy(GRID[x][y].TopTextureName, "");
            strcpy(GRID[x][y].BottomTextureName, "");
            GRID[x][y].BottomTextureType = MODEL_NONE;
            GRID[x][y].TopTextureType = MODEL_NONE;
        }
    }
}
