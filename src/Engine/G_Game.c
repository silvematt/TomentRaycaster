#include <math.h>

#include "I_InputHandling.h"
#include "G_Game.h"
#include "P_Physics.h"
#include "M_Map.h"
#include "G_Pathfinding.h"

// Game Timer
Timer* gameTimer;

// Current Game Time
double curTime = 0;

// Game Time at last tick
double oldTime = 0;

// Doors
int doorstateLevel0[MAP_HEIGHT][MAP_WIDTH];       // State of the door (open, closed, opening, closing)
int doorstateLevel1[MAP_HEIGHT][MAP_WIDTH];       // State of the door (open, closed, opening, closing)
int doorstateLevel2[MAP_HEIGHT][MAP_WIDTH];       // State of the door (open, closed, opening, closing)

float doorpositionsLevel0[MAP_HEIGHT][MAP_WIDTH]; // Timer holding the position of the door
float doorpositionsLevel1[MAP_HEIGHT][MAP_WIDTH]; // Timer holding the position of the door
float doorpositionsLevel2[MAP_HEIGHT][MAP_WIDTH]; // Timer holding the position of the door

// Dynamic AI list
sprite_t* allDynamicSprites[OBJECTARRAY_DEFAULT_SIZE];
unsigned int allDynamicSpritesLength = 0;

//-------------------------------------
// Initialize game related stuff 
//-------------------------------------
void G_InitGame(void)
{
    // Initialize game
    if(gameTimer == NULL)
        gameTimer = U_TimerCreateNew();

    gameTimer->Init(gameTimer);

    // Initialize Doors //
    memset(doorstateLevel0, 0, MAP_HEIGHT*MAP_WIDTH*sizeof(int));
    memset(doorstateLevel1, 0, MAP_HEIGHT*MAP_WIDTH*sizeof(int));
    memset(doorstateLevel2, 0, MAP_HEIGHT*MAP_WIDTH*sizeof(int));

    // All doors start closed
    for(int y = 0; y < MAP_HEIGHT; y++)
        for(int x = 0; x < MAP_WIDTH; x++)
        {
            doorpositionsLevel0[y][x] = DOOR_FULLY_CLOSED;
            doorpositionsLevel1[y][x] = DOOR_FULLY_CLOSED;
            doorpositionsLevel2[y][x] = DOOR_FULLY_CLOSED;
        }

    P_PhysicsInit();

    G_ChangeMap("devmap");
    
    gameTimer->Start(gameTimer);

}

//-------------------------------------
// Tick 
//-------------------------------------
void G_GameLoop(void)
{
    switch(application.gamestate)
    {
        case GSTATE_MENU:
            G_StateMenuLoop();
            break;

        case GSTATE_GAME:
            G_StateGameLoop();
            break;
    }
}


void G_StateGameLoop(void)
{
    curTime = gameTimer->GetTicks(gameTimer);
    
    P_PhysicsTick();

    // Handle input
    I_HandleInput();

    // Do stuff
    G_PlayerTick();
    G_UpdateDoors();
    G_UpdateAI();
    
    P_PhysicsEndTick();
    
    // Render
    // Creates the frame
    R_ComposeFrame();

    // Displays it on the screen
    R_FinishUpdate();

    oldTime = curTime;
}

void G_StateMenuLoop(void)
{
    // Handles input
    I_HandleInput();

    // Clears current render
    SDL_FillRect(win_surface, NULL, r_blankColor);

    // Creates the frame
    R_ComposeFrame();

    // Displays it on the screen
    R_FinishUpdate();
}

//-------------------------------------
// Update Doors by moving them in base of their timer
//-------------------------------------
void G_UpdateDoors(void)
{
    for(int y = 0; y < MAP_HEIGHT; y++)
        for(int x = 0; x < MAP_WIDTH; x++)
            {
                // LEVEL 0 
                // If the door is closed or open, continue
                if(doorstateLevel0[y][x] == DState_Closed || doorstateLevel0[y][x] == DState_Open)
                {
                    // continue
                }
                else
                {
                    // Open the door
                    if(doorstateLevel0[y][x] == DState_Opening)
                    {
                        if(doorpositionsLevel0[y][x] > DOOR_FULLY_OPENED &&
                            doorpositionsLevel0[y][x] - DOOR_OPEN_SPEED * deltaTime > DOOR_FULLY_OPENED) // check if a step is too big
                            doorpositionsLevel0[y][x] -= DOOR_OPEN_SPEED * deltaTime;
                        else
                        {
                            // Door opened
                            doorpositionsLevel0[y][x] = DOOR_FULLY_OPENED;
                            doorstateLevel0[y][x] = DState_Open;

                            // Update collision map
                            currentMap.collisionMapLevel0[y][x] = 0;
                        }
                    }
                    else if(doorstateLevel0[y][x] == DState_Closing)
                    {
                        if(doorpositionsLevel0[y][x] < DOOR_FULLY_CLOSED &&
                            doorpositionsLevel0[y][x] + DOOR_CLOSE_SPEED * deltaTime < DOOR_FULLY_CLOSED) // check if step is too big
                            doorpositionsLevel0[y][x] += DOOR_CLOSE_SPEED * deltaTime;
                        else
                        {
                            // Door closed
                            doorpositionsLevel0[y][x] = DOOR_FULLY_CLOSED;
                            doorstateLevel0[y][x] = DState_Closed;

                            // Update collision map
                            currentMap.collisionMapLevel0[y][x] = 1;
                        }
                    }
                }

                // LEVEL 1
                // If the door is closed or open, continue
                if(doorstateLevel1[y][x] == DState_Closed || doorstateLevel1[y][x] == DState_Open)
                {
                    // continue
                }
                else
                {
                    // Open the door
                    if(doorstateLevel1[y][x] == DState_Opening)
                    {
                        if(doorpositionsLevel1[y][x] > DOOR_FULLY_OPENED &&
                            doorpositionsLevel1[y][x] - DOOR_OPEN_SPEED * deltaTime > DOOR_FULLY_OPENED) // check if a step is too big
                            doorpositionsLevel1[y][x] -= DOOR_OPEN_SPEED * deltaTime;
                        else
                        {
                            // Door opened
                            doorpositionsLevel1[y][x] = DOOR_FULLY_OPENED;
                            doorstateLevel1[y][x] = DState_Open;

                            // Update collision map
                            currentMap.collisionMapLevel1[y][x] = 0;
                        }
                    }
                    else if(doorstateLevel1[y][x] == DState_Closing)
                    {
                        if(doorpositionsLevel1[y][x] < DOOR_FULLY_CLOSED &&
                            doorpositionsLevel1[y][x] + DOOR_CLOSE_SPEED * deltaTime < DOOR_FULLY_CLOSED) // check if step is too big
                            doorpositionsLevel1[y][x] += DOOR_CLOSE_SPEED * deltaTime;
                        else
                        {
                            // Door closed
                            doorpositionsLevel1[y][x] = DOOR_FULLY_CLOSED;
                            doorstateLevel1[y][x] = DState_Closed;

                            // Update collision map
                            currentMap.collisionMapLevel1[y][x] = 1;
                        }
                    }
                }

                // LEVEL 2
                // If the door is closed or open, continue
                if(doorstateLevel2[y][x] == DState_Closed || doorstateLevel2[y][x] == DState_Open)
                {
                    // continue
                }
                else
                {
                    // Open the door
                    if(doorstateLevel2[y][x] == DState_Opening)
                    {
                        if(doorpositionsLevel2[y][x] > DOOR_FULLY_OPENED &&
                            doorpositionsLevel2[y][x] - DOOR_OPEN_SPEED * deltaTime > DOOR_FULLY_OPENED) // check if a step is too big
                            doorpositionsLevel2[y][x] -= DOOR_OPEN_SPEED * deltaTime;
                        else
                        {
                            // Door opened
                            doorpositionsLevel2[y][x] = DOOR_FULLY_OPENED;
                            doorstateLevel2[y][x] = DState_Open;

                            // Update collision map
                            currentMap.collisionMapLevel2[y][x] = 0;
                        }
                    }
                    else if(doorstateLevel2[y][x] == DState_Closing)
                    {
                        if(doorpositionsLevel2[y][x] < DOOR_FULLY_CLOSED &&
                            doorpositionsLevel2[y][x] + DOOR_CLOSE_SPEED * deltaTime < DOOR_FULLY_CLOSED) // check if step is too big
                            doorpositionsLevel2[y][x] += DOOR_CLOSE_SPEED * deltaTime;
                        else
                        {
                            // Door closed
                            doorpositionsLevel2[y][x] = DOOR_FULLY_CLOSED;
                            doorstateLevel2[y][x] = DState_Closed;

                            // Update collision map
                            currentMap.collisionMapLevel2[y][x] = 1;
                        }
                    }
                }
            }
}

void G_ChangeMap(char* mapID)
{
    M_LoadMapAsCurrent(mapID);
    G_InitPlayer();
}


void G_UpdateAI(void)
{
    for(int y = 0; y < MAP_HEIGHT; y++)
        for(int x = 0; x < MAP_WIDTH; x++)
        {
            sprite_t* cur = currentMap.dynamicSprites[y][x];

            if(cur == NULL || cur->active == false)
                continue;


            int oldGridPosX = cur->gridPos.x;
            int oldGridPosY = cur->gridPos.y;

            // Calculate centered pos
            cur->centeredPos.x = cur->pos.x + (TILE_SIZE / 2);
            cur->centeredPos.y = cur->pos.y + (TILE_SIZE / 2);

            cur->gridPos.x = cur->centeredPos.x / TILE_SIZE;
            cur->gridPos.y = cur->centeredPos.y / TILE_SIZE;
            
            // Calculate runtime stuff
            // Get Player Space pos
            cur->pSpacePos.x = (cur->pos.x + (TILE_SIZE/2)) - player.centeredPos.x;
            cur->pSpacePos.y = (cur->pos.y + (TILE_SIZE/2)) - player.centeredPos.y;

            // Calculate the distance to player
            cur->dist = sqrt(cur->pSpacePos.x*cur->pSpacePos.x + cur->pSpacePos.y*cur->pSpacePos.y);

            // Set the target as the player
            cur->targetGridPos.x = player.gridPosition.x;
            cur->targetGridPos.y = player.gridPosition.y;

            path_t path = G_PerformPathfinding(cur->level, cur->gridPos, player.gridPosition, cur);
            cur->path = &path;

            bool moved = false;
            float deltaX = 0.0f;
            float deltaY = 0.0f; 

            if(path.isValid && path.nodesLength-1 > 0 && path.nodes[path.nodesLength-1] != NULL &&
               currentMap.dynamicSprites[path.nodes[path.nodesLength-1]->gridPos.y][path.nodes[path.nodesLength-1]->gridPos.x] == NULL)
            {
                deltaX = (path.nodes[path.nodesLength-1]->gridPos.x * TILE_SIZE + (TILE_SIZE/2)) - cur->centeredPos.x;
                deltaY = (path.nodes[path.nodesLength-1]->gridPos.y * TILE_SIZE + (TILE_SIZE/2)) - cur->centeredPos.y;

                if(P_CheckCircleCollision(&cur->collisionCircle, &player.collisionCircle) < 0 && 
                    P_GetDistance(player.centeredPos.x, player.centeredPos.y, cur->centeredPos.x + ((deltaX * cur->speed) * deltaTime), cur->centeredPos.y + ((deltaX * cur->speed) * deltaTime)) > TILE_SIZE)
                    {
                        cur->pos.x += (deltaX * cur->speed) * deltaTime;
                        cur->pos.y += (deltaY * cur->speed) * deltaTime; 

                        // Recalculate centered pos after delta move
                        cur->centeredPos.x = cur->pos.x + (TILE_SIZE / 2);
                        cur->centeredPos.y = cur->pos.y + (TILE_SIZE / 2);

                        cur->gridPos.x = cur->centeredPos.x / TILE_SIZE;
                        cur->gridPos.y = cur->centeredPos.y / TILE_SIZE;

                        moved = true;
                    }
            }


            // Check if this AI changed grid pos
            // TODO: make dynamicSprites map for each level
            if(!(oldGridPosX == cur->gridPos.x && oldGridPosY == cur->gridPos.y))
            {
                // If the tile the AI ended up in is not occupied
                if(currentMap.dynamicSprites[cur->gridPos.y][cur->gridPos.x] == NULL)
                {
                    // Update the dynamic map 
                    currentMap.dynamicSprites[cur->gridPos.y][cur->gridPos.x] = currentMap.dynamicSprites[y][x];
                    currentMap.dynamicSprites[y][x] = NULL;
                }
                else
                {
                    // Move back
                    cur->pos.x -= (deltaX * cur->speed) * deltaTime;
                    cur->pos.y -= (deltaY * cur->speed) * deltaTime; 

                    cur->gridPos.x = oldGridPosX;
                    cur->gridPos.y = oldGridPosY;
                }
            }
            

            cur->collisionCircle.pos.x = cur->centeredPos.x;
            cur->collisionCircle.pos.y = cur->centeredPos.y;
        }
        
}