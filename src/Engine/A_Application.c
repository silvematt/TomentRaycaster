#include "A_Application.h"

#include "R_Rendering.h"

// Definitions
app_t application;

SDL_Surface* win_surface;
int win_width;
unsigned int* pixels;

//-------------------------------------
// Initializes the application and subsystems
//-------------------------------------
void A_InitApplication(void)
{
    application.quit = false;

    printf("Booting up...\n");

    SDL_Init(SDL_INIT_EVERYTHING);

    uint32_t winFlags = 0; 
    application.win = SDL_CreateWindow("Toment", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, winFlags);
    win_surface = SDL_GetWindowSurface(application.win);

    win_width = win_surface->w;
    pixels = win_surface->pixels;

    // Init Renderer
    R_InitRendering();
}

//-------------------------------------
// Quit Applicaiton 
//-------------------------------------
void A_QuitApplication(void)
{
    SDL_Quit();
}