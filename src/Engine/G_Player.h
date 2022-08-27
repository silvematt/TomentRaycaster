#ifndef PLAYER_H_INCLUDED
#define PLAYER_H_INCLUDED

#include "A_Application.h"
#include "G_Game.h"
#include "U_DataTypes.h"

// Minimap Player
#define PLAYER_WIDTH 16
#define PLAYER_HEIGHT 16

#define PLAYER_CENTER_FIX 8     // To fix the player position to be at the center instead of the upper corner, this is WIDTH/2

#define PLAYER_SPEED 250.0f
#define PLAYER_ROT_SPEED 1.5f

#define PLAYER_STARTING_GRID_X 12
#define PLAYER_STARTING_GRID_Y 12
#define PLAYER_STARTING_ROT M_PI

#define PLAYER_STARTING_X PLAYER_STARTING_GRID_X*TILE_SIZE
#define PLAYER_STARTING_Y PLAYER_STARTING_GRID_Y*TILE_SIZE

// The minimum distance the player can get to the wall
#define PLAYER_MIN_DIST_TO_WALL 30

// The light that the player will always carry, no matter the current map
#define PLAYER_POINT_LIGHT_INTENSITY 10


// ----------------------------------------------------
// Sets an SDL_Rect
// ----------------------------------------------------
typedef struct player_s
{
    vector2_t position;     // abs SDL position
    vector2_t centeredPos;  // position centered for the player's width and height
    
    vector2Int_t gridPosition;  // position of the player in the tilemap
    vector2Int_t inFrontGridPosition; // The grid pos of the cell in front of the player
    vector2_t deltaPos; // used to calculate movements and collision

    // Minimap rapresentaton
    SDL_Surface* surface;
    SDL_Rect surfaceRect;

    // Rot of the player in radians
    float angle;
} player_t;


extern player_t player;

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
void G_InGameInputHandling(const uint8_t* keyboardState, SDL_Event* e);

//-------------------------------------
// Handles Input from the player while doing the Event Input Handling
//-------------------------------------
void G_InGameInputHandlingEvent(SDL_Event* e);

#endif