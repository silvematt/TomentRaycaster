#include "A_Application.h"

#include "R_Rendering.h"

// Definitions
app_t application;

SDL_Surface* win_surface;
int win_width;
unsigned int* pixels;

// Game State
bool isInMenu;       // True if the player is in a menu
bool isInGame;       // True if the player is in game

//-------------------------------------
// Initializes the application and subsystems
//-------------------------------------
void A_InitApplication(void)
{
    application.quit = false;

    printf("Booting up...\n");

    SDL_Init(SDL_INIT_EVERYTHING);

    //uint32_t winFlags = SDL_WINDOW_FULLSCREEN; 
    uint32_t winFlags = 0; 

    application.win = SDL_CreateWindow("Toment", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, winFlags);
    win_surface = SDL_GetWindowSurface(application.win);

    win_width = win_surface->w;
    pixels = win_surface->pixels;
    
    // Init Renderer
    R_InitRendering();

    srand(time(NULL));

    // Define entry state for the application
    A_ChangeState(GSTATE_MENU);
}

//-------------------------------------
// Engine tick 
//-------------------------------------
void A_EngineLoop(void)
{
    
}

//-------------------------------------
// Quit Applicaiton 
//-------------------------------------
void A_QuitApplication(void)
{
    SDL_Quit();
}

//-------------------------------------
// Change State 
//-------------------------------------
void A_ChangeState(gamestate_e newState)
{
    application.gamestate = newState;

    switch(application.gamestate)
    {
        case GSTATE_MENU:
            SDL_SetRelativeMouseMode(SDL_FALSE);
            break;

        case GSTATE_GAME:
            SDL_SetRelativeMouseMode(SDL_TRUE);
            break;
    }
}