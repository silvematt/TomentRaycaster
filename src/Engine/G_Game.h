#ifndef GAME_H_INCLUDED
#define GAME_H_INCLUDED

#include "G_Player.h"
#include "R_Rendering.h"
#include "U_Timer.h"

// DOOR DEFINE
#define DOOR_FULLY_CLOSED 64.0f
#define DOOR_FULLY_OPENED 2.0f
#define DOOR_OPEN_SPEED 100.0f
#define DOOR_CLOSE_SPEED 100.0f

// Game Timer
extern Timer* gameTimer;

// Current Game Time
extern double curTime;

// Game Time at last tick
extern double oldTime;

// Doors
extern int doorstateLevel0[MAP_HEIGHT][MAP_WIDTH];       // State of the door (open, closed, opening, closing)
extern int doorstateLevel1[MAP_HEIGHT][MAP_WIDTH];       // State of the door (open, closed, opening, closing)
extern int doorstateLevel2[MAP_HEIGHT][MAP_WIDTH];       // State of the door (open, closed, opening, closing)

extern float doorpositionsLevel0[MAP_HEIGHT][MAP_WIDTH]; // Timer holding the position of the door
extern float doorpositionsLevel1[MAP_HEIGHT][MAP_WIDTH]; // Timer holding the position of the door
extern float doorpositionsLevel2[MAP_HEIGHT][MAP_WIDTH]; // Timer holding the position of the door

// Dynamic AI list
extern sprite_t* allDynamicSprites[OBJECTARRAY_DEFAULT_SIZE];
extern unsigned int allDynamicSpritesLength;

//-------------------------------------
// Initialize game related stuff 
//-------------------------------------
void G_InitGame(void);

//-------------------------------------
// Abs Tick 
//-------------------------------------
void G_GameLoop(void);

void G_StateMenuLoop(void);

void G_StateGameLoop(void);

//-------------------------------------
// Update Doors by moving them in base of their timer
//-------------------------------------
void G_UpdateDoors(void);

void G_ChangeMap(char* mapID);

void G_UpdateAI(void);

#endif