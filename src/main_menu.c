#include <stdio.h>
#include <unistd.h>

#include "simple_logger.h"

#include "gfc_types.h"
#include "gfc_input.h"

#include "gf2d_windows.h"
#include "gf2d_elements.h"
#include "gf2d_element_label.h"
#include "gf2d_draw.h"
#include "gf2d_font.h"
#include "gf2d_mouse.h"
#include "gf2d_windows_common.h"

#include "main_menu.h"
#include "gf3d_vgraphics.h"

extern State CurrentState;

int LOADING = 0;
extern int _done;
Window *win;
typedef struct
{
    int selectedOption;
    Sprite *background;
    TextLine filename;
    Window *win;
} MainMenuData;

int main_menu_free(Window *win)
{
    MainMenuData *data;
    if (!win)
        return 0;
    if (!win->data)
        return 0;
    data = win->data;
    gf2d_sprite_free(data->background);
    free(data);

    return 0;
}

int main_menu_draw(Window *win)
{
    MainMenuData *data;
    if (!win)
        return 0;
    if (!win->data)
        return 0;
    data = win->data;
    int imageWidth = 1920, imageHeight = 1080;
    Vector2D scale = vector2d((float)gf3d_vgraphics_get_width() / imageWidth, (float)gf3d_vgraphics_get_height() / imageHeight);
    gf2d_sprite_draw_image(data->background, vector2d(0, 0), &scale);
    return 0;
}

int main_menu_update(Window *win, List *updateList)
{
    int i, count;
    Element *e;
    MainMenuData *data;
    if (!win)
        return 0;
    if (!updateList)
        return 0;
    data = (MainMenuData *)win->data;
    if (!data)
        return 0;

    if (gfc_input_command_pressed("nextelement"))
    {
        gf2d_window_next_focus(win);
        return 1;
    }

    count = gfc_list_get_count(updateList);
    for (i = 0; i < count; i++)
    {
        e = gfc_list_get_nth(updateList, i);
        if (!e)
            continue;
        if (strcmp(e->name, "newgame") == 0)
        {
            CurrentState = SINGLEPLAYER_GAME;
            LOADING = 1;
            return 1;
        }
        else if (strcmp(e->name, "multiplayer") == 0)
        {
            CurrentState = MULTIPLAYER_GAME;
            LOADING = 1;
            return 1;
        }
        else if (strcmp(e->name, "contenteditor") == 0)
        {
            CurrentState = CONTENT_EDITOR_GAME;
            LOADING = 1;
            return 1;
        }
        else if (strcmp(e->name, "quit") == 0)
        {
            _done = 1;
            return 1;
        }
    }
    if (CurrentState == LOADED_GAME)
    {
        LOADING = 0;
        gf2d_window_free(win);
    }
    return 0;
}

Window *main_menu()
{

    MainMenuData *data;
    win = gf2d_window_load("assets/menus/main_menu.json");
    if (!win)
    {
        slog("failed to load editor menu");
        return NULL;
    }
    win->update = main_menu_update;
    win->free_data = main_menu_free;
    win->draw = main_menu_draw;
    data = (MainMenuData *)gfc_allocate_array(sizeof(MainMenuData), 1);
    data->background = gf2d_sprite_load_image("assets/images/ui/wallpaper.jpg");
    gf2d_window_set_focus_to(win, gf2d_window_get_element_by_name(win, "newgame"));
    win->data = data;
    data->win = win;
    return win;
}

void main_menu_change_value()
{
    gf2d_element_label_set_text(gf2d_window_get_element_by_id(win, 101), "Continue");
}

/*eol@eof*/
