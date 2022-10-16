#include <math.h>

#include "I_InputHandling.h"
#include "G_Game.h"
#include "P_Physics.h"
#include "M_Map.h"
#include "G_Pathfinding.h"
#include "G_AI.h"

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

// Projectiles travelling in the world
projectileNode_t* projectilesHead = NULL;

// Projectiles that hit something and are exploding
projectileNode_t* explodingProjectilesHead = NULL;

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

    // Empty projectile list
    if(projectilesHead != NULL)
    {
        projectileNode_t* current = projectilesHead;

        while(current != NULL)
        {
            projectileNode_t* tmp = current;
            current = current->next;
            free(tmp);
        }
        
        projectilesHead = NULL;
    }

    P_PhysicsInit();

    G_ChangeMap("devmap");
    
    gameTimer->Start(gameTimer);
}

//-------------------------------------
// Game Tick 
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
    G_AIUpdate();
    G_UpdateProjectiles();
    
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

void G_UpdateProjectiles(void)
{   
    projectileNode_t* cur = projectilesHead;
    int i = 0;
    while(cur != NULL)
    {
        // Update base info needed for rendering
        cur->this.base.centeredPos.x = cur->this.base.pos.x;
        cur->this.base.centeredPos.y = cur->this.base.pos.y;

        cur->this.base.pSpacePos.x = cur->this.base.centeredPos.x - player.centeredPos.x;
        cur->this.base.pSpacePos.y = cur->this.base.centeredPos.y - player.centeredPos.y;

        cur->this.base.gridPos.x = cur->this.base.centeredPos.x / TILE_SIZE;
        cur->this.base.gridPos.y = cur->this.base.centeredPos.y / TILE_SIZE;

        cur->this.base.dist = sqrt(cur->this.base.pSpacePos.x*cur->this.base.pSpacePos.x + cur->this.base.pSpacePos.y*cur->this.base.pSpacePos.y);

        // If the projectile hasnt hit anything, check for hit
        if(!cur->this.isBeingDestroyed)
        {
            // Update pos
            cur->this.base.pos.x += cos(cur->this.base.angle) * cur->this.speed * deltaTime;
            cur->this.base.pos.y += sin(cur->this.base.angle) * cur->this.speed * deltaTime;

            // Check if projectile is not out of map
            bool insideMap = cur->this.base.gridPos.x >= 0 && cur->this.base.gridPos.y >= 0 && cur->this.base.gridPos.x < MAP_WIDTH && cur->this.base.gridPos.y < MAP_HEIGHT;

            // Destroy condition
            if(G_CheckCollisionMap(cur->this.base.level, cur->this.base.gridPos.y, cur->this.base.gridPos.x) == 1 || !insideMap)
            {
                cur->this.isBeingDestroyed = true;
                G_AIPlayAnimationOnce(&cur->this, ANIM_DIE);
                return;
            }

            dynamicSprite_t* sprite = NULL;
            if((sprite = G_GetFromDynamicSpriteMap(cur->this.base.level, cur->this.base.gridPos.y, cur->this.base.gridPos.x)) != NULL)
            {
                float damage = 0.0f;

                // Damage sprite
                switch(cur->this.base.spriteID)
                {
                    case S_Fireball1:
                        damage = 55.65f;
                        break;

                    case S_IceDart1:
                        damage = 10.0f;
                        break;

                    default:
                        damage = 0.0f;
                        break;
                }

                if(cur->this.isOfPlayer)
                {
                    player.crosshairHit = true;
                    player.crosshairTimer->Start(player.crosshairTimer);
                }

                G_AITakeDamage(sprite, damage);

                cur->this.isBeingDestroyed = true;
                G_AIPlayAnimationOnce(&cur->this, ANIM_DIE);
                return;
            }
        }
        // The projectile has hit, wait for the death animation to play
        else
        {
            if(cur->this.animFrame >= tomentdatapack.sprites[cur->this.base.spriteID]->animations->animDieSheetLength-1)
            {
                // Destroy this
                if(projectilesHead == cur)
                    projectilesHead = cur->next;

                if(cur->next != NULL)
                    cur->next->previous = cur->previous;

                if(cur->previous != NULL)
                    cur->previous->next = cur->next;

                free(cur->this.animTimer);
                free(cur);
            }
        }

        // Select Animation & Play it
        switch(cur->this.state)
        {
            case DS_STATE_IDLE:
                cur->this.curAnim = tomentdatapack.sprites[cur->this.base.spriteID]->animations->animIdle;
                cur->this.curAnimLength = tomentdatapack.sprites[cur->this.base.spriteID]->animations->animIdleSheetLength;
                break;

            case DS_STATE_DEAD:
                cur->this.curAnim = tomentdatapack.sprites[cur->this.base.spriteID]->animations->animDie;
                cur->this.curAnimLength = tomentdatapack.sprites[cur->this.base.spriteID]->animations->animDieSheetLength;
                break;

            case DS_STATE_ATTACKING:
                cur->this.curAnim = tomentdatapack.sprites[cur->this.base.spriteID]->animations->animAttack;
                cur->this.curAnimLength = tomentdatapack.sprites[cur->this.base.spriteID]->animations->animAttackSheetLength;
                break;

            default:
                cur->this.curAnim = tomentdatapack.sprites[cur->this.base.spriteID]->animations->animIdle;
                cur->this.curAnimLength = tomentdatapack.sprites[cur->this.base.spriteID]->animations->animIdleSheetLength;
                break;
        }

        if(cur->this.animPlay)
        {
            if(cur->this.animPlayOnce)
            {
                if(cur->this.curAnimLength > 0)
                    cur->this.animFrame = ((int)floor(cur->this.animTimer->GetTicks(cur->this.animTimer) / cur->this.animSpeed) % cur->this.curAnimLength);

                // Prevent loop
                if(cur->this.animFrame >= cur->this.curAnimLength-1)
                {
                    cur->this.animPlay = false;

                    // Go back to idle if it was attacking
                    if(cur->this.state == DS_STATE_ATTACKING)
                    {
                        G_AIPlayAnimationLoop(&cur->this, ANIM_IDLE);
                    }
                }
            }
            else
            {
                // Allow loop
                if(cur->this.curAnimLength > 0)
                    cur->this.animFrame = ((int)floor(cur->this.animTimer->GetTicks(cur->this.animTimer) / cur->this.animSpeed) % cur->this.curAnimLength);
            }
        }

        i++;
        cur = cur->next;
    }
}


void G_SpawnProjectile(int id, float angle, int level, float posx, float posy, bool isOfPlayer)
{
    // Allocate a node
    projectileNode_t* newNode = (projectileNode_t*)malloc(sizeof(projectileNode_t));

    // Set initial data like pos, dir and speed
    newNode->this.type = DS_TYPE_PROJECTILE;
    newNode->this.curAnim = NULL;
    newNode->this.curAnimLength = 0;
    newNode->this.animPlay = true;
    newNode->this.state = ANIM_IDLE;
    newNode->this.animTimer = U_TimerCreateNew();
    newNode->this.isBeingDestroyed = false;

    newNode->this.base.spriteID = id;
    newNode->this.base.sheetLength = tomentdatapack.spritesSheetsLenghtTable[id];
    newNode->this.speed = 500.0f;

    newNode->this.base.active = true;
    newNode->this.base.angle = angle;
    newNode->this.base.level = level;

    newNode->this.base.pos.x = posx;
    newNode->this.base.pos.y = posy;

    newNode->this.base.gridPos.x = posx / TILE_SIZE;
    newNode->this.base.gridPos.y = posy / TILE_SIZE;

    newNode->this.base.centeredPos.x = newNode->this.base.pos.x + (TILE_SIZE / 2);
    newNode->this.base.centeredPos.y = newNode->this.base.pos.y + (TILE_SIZE / 2);
    
    newNode->this.base.pSpacePos.x = newNode->this.base.centeredPos.x - player.centeredPos.x;
    newNode->this.base.pSpacePos.y = newNode->this.base.centeredPos.y - player.centeredPos.y;

    newNode->this.isOfPlayer = isOfPlayer;
    
    newNode->next = NULL;
    if(projectilesHead == NULL)
    {
        projectilesHead = newNode;
        projectilesHead->next = NULL;
        projectilesHead->previous = NULL;
    }
    else
    {
        projectileNode_t* current = projectilesHead;

        while(current->next != NULL)
            current = current->next;

        // Now we can add
        current->next = newNode;
        current->next->next = NULL;
        newNode->previous = current;
    }

    // Play idle anim
    G_AIPlayAnimationLoop(&newNode->this, ANIM_IDLE);
}