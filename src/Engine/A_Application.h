#ifndef APPLICATION_H_INCLUDED
#define APPLICATION_H_INCLUDED

#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <stdlib.h>

#include "../include/SDL2/SDL.h"

// Defines
#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

#define MAX_STRLEN 255      // Max lenght of a filepath

#define MAX_FILEPATH_L 255  // Max lenght of a filepath
#define MAX_STRL_R 8192      // Max lenght of a fgets while reading


// Holds the current game state
typedef enum gamestate_e
{
    dev = 0,
    GSTATE_MENU,
    GSTATE_GAME
} gamestate_e;

// Fundamental information about the application
typedef struct app_s
{
    SDL_Window* win;
    bool quit;
    gamestate_e gamestate;
} app_t;

// Declarations
extern app_t application;

extern SDL_Surface* win_surface;    // the surface of the window        
extern int win_width;               // win_surface->w
extern unsigned int* pixels;        // pixels of the surface

//-------------------------------------
// Initializes the application 
//-------------------------------------
void A_InitApplication(void);

//-------------------------------------
// Tick 
//-------------------------------------
void A_EngineLoop(void);

//-------------------------------------
// Quit Application 
//-------------------------------------
void A_QuitApplication(void);

//-------------------------------------
// Change State 
//-------------------------------------
void A_ChangeState(gamestate_e newState);

#endif