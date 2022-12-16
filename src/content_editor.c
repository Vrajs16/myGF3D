#include "gf3d_vgraphics.h"
#include "gf2d_font.h"
#include "gf2d_draw.h"
#include "content_editor.h"

SDL_Renderer *renderer = NULL;

SDL_Color grid_background = {22, 22, 22, 255}; // Barely Black
SDL_Color grid_line_color = {44, 44, 44, 255}; // Dark grey
SDL_Color grid_cursor_ghost_color = {44, 44, 44, 255};
SDL_Color grid_cursor_color = {255, 255, 255, 255}; // White

void content_editor_setup_renderer()
{
    renderer = gf3d_vgraphics_get_renderer();
    SDL_ShowCursor(SDL_ENABLE);
}

void content_editor_draw()
{
    // Render with no flickering
    SDL_SetRenderDrawColor(renderer, grid_background.r, grid_background.g, grid_background.b, grid_background.a);
    SDL_RenderClear(renderer);

    // Draw filled rectangle
    SDL_Rect rect = {0, 0, 100, 100};
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderFillRect(renderer, &rect);

    // present
    SDL_RenderPresent(renderer);
};
