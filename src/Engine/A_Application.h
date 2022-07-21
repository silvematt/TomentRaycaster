#ifndef APPLICATION_H_INCLUDED
#define APPLICATION_H_INCLUDED

#include <stdio.h>
#include <stdbool.h>

#include "../include/SDL2/SDL.h"

// Defines
#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

#define MAX_STRLEN 255  // Max lenght of a filepath

#define MAX_FILEPATH_L 255  // Max lenght of a filepath
#define MAX_STRL_R 255      // Max lenght of a fgets while reading


typedef enum gamestate_e
{
    dev = 0
} gamestate_t;

typedef struct app_s
{
    SDL_Window* win;
    bool quit;
    gamestate_t gamestate;
} app_t;

// Declarations
extern app_t application;

extern SDL_Surface* win_surface;
extern int win_width;
extern unsigned int* pixels;


//-------------------------------------
// Initializes the application 
//-------------------------------------
void A_InitApplication(void);

//-------------------------------------
// Tick 
//-------------------------------------
void A_GameLoop(void);

//-------------------------------------
// Quit Applicaiton 
//-------------------------------------
void A_QuitApplication(void);

#endif