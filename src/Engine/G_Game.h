#ifndef GAME_H_INCLUDED
#define GAME_H_INCLUDED

#include "G_Player.h"
#include "R_Rendering.h"

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

#endif