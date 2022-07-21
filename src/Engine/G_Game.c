#include "I_InputHandling.h"
#include "G_Game.h"
#include "G_Physics.h"
#include "M_Map.h"

// Current Game Time
double curTime = 0;

// Game Time at last tick
double oldTime = 0;

//-------------------------------------
// Initialize game related stuff 
//-------------------------------------
void G_InitGame(void)
{
    G_PhysicsInit();

    M_LoadMapAsCurrent("devmap");

    G_InitPlayer();
}

//-------------------------------------
// Tick 
//-------------------------------------
void G_GameLoop(void)
{
    // Handle input
    I_HandleInput();

    G_PhysicsTick();

    // Do stuff
    G_PlayerTick();

    G_PhysicsEndTick();
    
    // Render
    // Creates the frame
    R_ComposeFrame();

    // Sends it to the win_surface
    R_UpdateNoBlit();

    // Displays it on the screen
    R_FinishUpdate();
}