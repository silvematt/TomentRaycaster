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
#define PLAYER_ROT_SPEED 0.5f

#define PLAYER_STARTING_ROT M_PI

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
    float z;
    int level;              // The current floor level the player is at
    
    circle_t collisionCircle;

    vector2Int_t gridPosition;  // position of the player in the tilemap
    vector2Int_t inFrontGridPosition; // The grid pos of the cell in front of the player
    vector2_t deltaPos; // used to calculate movements and collision

    // Minimap rapresentaton
    SDL_Surface* surface;
    SDL_Rect surfaceRect;

    // Rot of the player in radians
    float angle;

    // Loaded at runtime
    // The grid the player starts in (loaded from map)
    int startingGridX; 
    int startingGridY;
    float startingRot;
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

//-------------------------------------
// Checks the collision map at player's level and returns what found
//-------------------------------------
int G_CheckCollisionMap(int level, int y, int x);

//-------------------------------------
// Checks door state map at player's level and returns what found
//-------------------------------------
int G_GetDoorState(int level, int y, int x);

//-------------------------------------
// Sets door state map at player's level
//-------------------------------------
int G_SetDoorState(int level, int y, int x, doorstate_e state);

//-------------------------------------
// Checks door position map at player's level and returns what found
//-------------------------------------
float G_GetDoorPosition(int level, int y, int x);

//-------------------------------------
// Checks object T map at player's level and returns what found
//-------------------------------------
int G_GetFromObjectTMap(int level, int y, int x);

void G_PlayerCollisionCheck();

void G_PlayerRender(void);

#endif