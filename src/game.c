#include <SDL.h>
#include "simple_logger.h"
#include "gfc_input.h"
#include "gameloop.h"

extern int __DEBUG;
int __BB = 0;

int _done = 0;

static const double FRAMEDELAY = 1000.0 / 60.0;
int MULTIPLAYER = 0;

void handle_sigint(int sig)
{
    _done = 1;
}

int main(int argc, char *argv[])
{
    signal(SIGINT, handle_sigint);
    int a;
    for (a = 1; a < argc; a++)
        if (strcmp(argv[a], "--debug") == 0)
            __DEBUG = 1;
        else if (strcmp(argv[a], "--bb") == 0 || strcmp(argv[a], "--BB") == 0 || strcmp(argv[a], "-bb") == 0 || strcmp(argv[a], "-BB") == 0)
            __BB = 1;
        else if (strcmp(argv[a], "--multiplayer") == 0 || strcmp(argv[a], "--MULTIPLAYER") == 0 || strcmp(argv[a], "-multiplayer") == 0 || strcmp(argv[a], "-MULTIPLAYER") == 0)
            MULTIPLAYER = 1;

    gameloop_setup();

    Uint32 frameStart;
    double frameTime;

    while (!_done)
    {
        frameStart = SDL_GetTicks();

        if (gfc_input_command_down("exit"))
            _done = 1;

        gameloop_update();
        gameloop_draw();

        frameTime = (double)SDL_GetTicks() - frameStart;
        if (FRAMEDELAY > frameTime)
            SDL_Delay(FRAMEDELAY - frameTime);
    }

    gameloop_close();

    return 0;
}
/*eol@eof*/
