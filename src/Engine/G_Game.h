#ifndef GAME_H_INCLUDED
#define GAME_H_INCLUDED

#include "G_Player.h"
#include "R_Rendering.h"

// DOOR DEFINE
#define DOOR_FULLY_CLOSED 64.0f
#define DOOR_FULLY_OPENED 2.0f
#define DOOR_OPEN_SPEED 100.0f
#define DOOR_CLOSE_SPEED 100.0f

// Current Game Time
extern double curTime;

// Game Time at last tick
extern double oldTime;

// Doors
extern int doorstate[MAP_HEIGHT][MAP_WIDTH];       // State of the door (open, closed, opening, closing)
extern float doorpositions[MAP_HEIGHT][MAP_WIDTH]; // Timer holding the position of the door

//-------------------------------------
// Initialize game related stuff 
//-------------------------------------
void G_InitGame(void);

//-------------------------------------
// Tick 
//-------------------------------------
void G_GameLoop(void);

//-------------------------------------
// Update Doors by moving them in base of their timer
//-------------------------------------
void G_UpdateDoors(void);

#endif