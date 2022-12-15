#ifndef __MAIN_MENU_H__
#define __MAIN_MENU_H__

#include "gf2d_windows.h"

typedef enum
{
    MAIN_MENU,
    SINGLEPLAYER_GAME,
    MULTIPLAYER_GAME,
    CONTENT_EDITOR_GAME,
    LOADED_GAME,
} State;

Window *main_menu();

#endif
