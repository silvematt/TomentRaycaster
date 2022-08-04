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

    G_UpdateDoors();

    G_PhysicsEndTick();
    
    // Render
    // Creates the frame
    R_ComposeFrame();

    // Sends it to the win_surface
    R_UpdateNoBlit();

    // Displays it on the screen
    R_FinishUpdate();
}

void G_UpdateDoors(void)
{
    for(int y = 0; y < MAP_HEIGHT; y++)
        for(int x = 0; x < MAP_WIDTH; x++)
            {
                // If the door is closed or open, continue
                if(doorstate[y][x] == DState_Closed || doorstate[y][x] == DState_Open)
                    continue;
                else
                {
                    // Open the door
                    if(doorstate[y][x] == DState_Opening)
                    {
                        if(doorpositions[y][x] > DOOR_FULLY_OPENED)
                            doorpositions[y][x] -= DOOR_OPEN_SPEED * deltaTime;
                        else
                        {
                            // Door opened
                            doorpositions[y][x] = DOOR_FULLY_OPENED;
                            doorstate[y][x] = DState_Open;

                            // Update collision map
                            currentMap.collisionMap[y][x] = 0;
                        }
                    }
                    else if(doorstate[y][x] == DState_Closing)
                    {
                        if(doorpositions[y][x] < DOOR_FULLY_CLOSED)
                            doorpositions[y][x] += DOOR_CLOSE_SPEED * deltaTime;
                        else
                        {
                            // Door closed
                            doorpositions[y][x] = DOOR_FULLY_CLOSED;
                            doorstate[y][x] = DState_Closed;

                            // Update collision map
                            currentMap.collisionMap[y][x] = 1;
                        }
                    }
                }
            }
}