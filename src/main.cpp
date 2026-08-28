#include "Core/Game.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

int main(int argc, char* argv[])
{
    Game game;
    if (game.init())
    {
        game.run();
    }
    return 0;
}
