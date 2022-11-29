#define TICK_INTERVAL 10
#include <SDL.h>
#include "simple_logger.h"
#include "gfc_input.h"
#include "gameloop.h"

extern int __DEBUG;
int __BB = 0;

static int _done = 0;
static Uint32 next_time = 0;

// 60 fps i think
Uint32 time_left(void)
{
    Uint32 now;
    now = SDL_GetTicks();

    if (next_time <= now)
        return 0;
    else
        return next_time - now;
}

int main(int argc, char *argv[])
{
    int a;
    for (a = 1; a < argc; a++)
        if (strcmp(argv[a], "--debug") == 0)
            __DEBUG = 1;
        else if (strcmp(argv[a], "--bb") == 0 || strcmp(argv[a], "--BB") == 0 || strcmp(argv[a], "-bb") == 0 || strcmp(argv[a], "-BB") == 0)
            __BB = 1;

    next_time = SDL_GetTicks() + TICK_INTERVAL;

    gameloop_setup();

    while (!_done)
    {
        gameloop_update();

        gameloop_draw();

        if (gfc_input_command_down("exit"))
            _done = 1;

        SDL_Delay(time_left());
        next_time += TICK_INTERVAL;
    }

    gameloop_close();

    return 0;
}

// static Window *_quit = NULL;
// // Quit the game
// void onCancel(void *data);
// void onExit(void *data);
// void exitGame();
// void exitCheck();

// void onCancel(void *data)
// {
//     _quit = NULL;
// }

// void onExit(void *data)
// {
//     _done = 1;
//     _quit = NULL;
// }

// void exitGame()
// {
//     _done = 1;
// }

// void exitCheck()
// {
//     if (_quit)
//         return;
//     _quit = window_yes_no("Exit?", onExit, onCancel, NULL);
// }
/*eol@eof*/
