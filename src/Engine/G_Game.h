#ifndef GAME_H_INCLUDED
#define GAME_H_INCLUDED

#include "G_Player.h"
#include "R_Rendering.h"

#define DOOR_FULLY_CLOSED 64.0f
#define DOOR_FULLY_OPENED 2.0f
#define DOOR_OPEN_SPEED 100.0f
#define DOOR_CLOSE_SPEED 100.0f

// Current Game Time
extern double curTime;

// Game Time at last tick
extern double oldTime;

//-------------------------------------
// Initialize game related stuff 
//-------------------------------------
void G_InitGame(void);

//-------------------------------------
// Tick 
//-------------------------------------
void G_GameLoop(void);

//-------------------------------------
// Update Doors 
//-------------------------------------
void G_UpdateDoors(void);

#endif