#ifndef PLAYER_H_INCLUDED
#define PLAYER_H_INCLUDED

#include "A_Application.h"
#include "U_DataTypes.h"

#define PLAYER_WIDTH 16
#define PLAYER_HEIGHT 16
#define PLAYER_CENTER_FIX 8     // To fix the player position to be at the center instead of the upper corner, this is WIDTH/2

#define PLAYER_SPEED 250.0f
#define PLAYER_ROT_SPEED 1.5f

#define PLAYER_MIN_DIST_TO_WALL 30

// The light that the player will always carry, no matter the current map
#define PLAYER_POINT_LIGHT_INTENSITY 10

typedef struct player_s
{
    vector2_t position;
    vector2_t centeredPos;
    
    vector2Int_t gridPosition;
    vector2Int_t inFrontGridPosition; // The grid pos of the cell in front of the player
    vector2_t deltaPos;

    SDL_Surface* surface;
    SDL_Rect surfaceRect;

    float angle;
} player_t;

extern player_t player;
extern SDL_Surface* playerSurface;

//-------------------------------------
// Initializes Player 
//-------------------------------------
void G_InitPlayer(void);

//-------------------------------------
// Player's Tick 
//-------------------------------------
void G_PlayerTick(void);

//-------------------------------------
// Handles Input from the player while reading the keyboard state
//-------------------------------------
void G_PlayerHandleInput(const uint8_t* keyboardState, SDL_Event* e);

//-------------------------------------
// Handles Input from the player while doing the Event Input Handling
//-------------------------------------
void G_PlayerHandleInputEvent(SDL_Event* e);

#endif