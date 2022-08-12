#include "I_InputHandling.h"
#include "G_Game.h"
#include "G_Physics.h"
#include "M_Map.h"

// Game Timer
Timer* gameTimer;

// Current Game Time
double curTime = 0;

// Game Time at last tick
double oldTime = 0;

// Doors
int doorstate[MAP_HEIGHT][MAP_WIDTH];       // State of the door (open, closed, opening, closing)
float doorpositions[MAP_HEIGHT][MAP_WIDTH]; // Timer holding the position of the door

//-------------------------------------
// Initialize game related stuff 
//-------------------------------------
void G_InitGame(void)
{
    // Initialize game
    gameTimer = U_TimerCreateNew();
    gameTimer->Init(gameTimer);

    // Initialize the rest
    G_PhysicsInit();

    M_LoadMapAsCurrent("devmap");

    G_InitPlayer();

    gameTimer->Start(gameTimer);
}

//-------------------------------------
// Tick 
//-------------------------------------
void G_GameLoop(void)
{
    curTime = gameTimer->GetTicks(gameTimer);
    
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

    oldTime = curTime;
}

//-------------------------------------
// Update Doors by moving them in base of their timer
//-------------------------------------
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
                        if(doorpositions[y][x] > DOOR_FULLY_OPENED &&
                            doorpositions[y][x] - DOOR_OPEN_SPEED * deltaTime > DOOR_FULLY_OPENED) // check if a step is too big
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
                        if(doorpositions[y][x] < DOOR_FULLY_CLOSED &&
                            doorpositions[y][x] + DOOR_CLOSE_SPEED * deltaTime < DOOR_FULLY_CLOSED) // check if step is too big
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