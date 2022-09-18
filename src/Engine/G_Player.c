#include <math.h>

#include "G_Player.h"
#include "R_Rendering.h"
#include "I_InputHandling.h"
#include "M_Map.h"
#include "G_Physics.h"
#include "U_Utilities.h"
#include "D_AssetsManager.h"
#include "G_Physics.h"

player_t player;    // Player

// ----------------------------------------------------
// Sets an SDL_Rect
// ----------------------------------------------------
void SDL_Rect_Set(SDL_Rect* r, int x, int y, int w, int h)
{
    r->x = x;
    r->y = y;
    r->w = w;
    r->h = h;
}

//-------------------------------------
// Initializes Player 
//-------------------------------------
void G_InitPlayer(void)
{
    // Init player
    player.position.x = (currentMap.playerStartingGridX * TILE_SIZE);
    player.position.y = (currentMap.playerStartingGridY * TILE_SIZE);

    player.collisionCircle.pos.x = player.position.x;
    player.collisionCircle.pos.y = player.position.y;

    player.angle = currentMap.playerStartingRot;
    player.z = (TILE_SIZE/2) + ((TILE_SIZE) * currentMap.playerStartingLevel);
    player.level = currentMap.playerStartingLevel;
    player.gridPosition.x = currentMap.playerStartingGridX;
    player.gridPosition.y = currentMap.playerStartingGridY;
    player.collisionCircle.r = 32;

    // Rect for minimap
    SDL_Rect_Set(&player.surfaceRect, (int)player.position.x, (int)player.position.y, PLAYER_WIDTH, PLAYER_HEIGHT);

    // Do one tick
    G_PlayerTick();
}

//-------------------------------------
// Player's Tick 
//-------------------------------------
void G_PlayerTick(void)
{
    // Get player grid pos
    player.gridPosition.x = ((player.position.x+PLAYER_CENTER_FIX) / TILE_SIZE);
    player.gridPosition.y = ((player.position.y+PLAYER_CENTER_FIX) / TILE_SIZE);
    
    //player.angle = M_PI / 4;
    player.angle += (playerinput.input.x * PLAYER_ROT_SPEED) * deltaTime;
    playerinput.input.x = 0; // kill the mouse movement after applying rot
    
    FIX_ANGLES(player.angle);

    playerinput.dir.x = cos(player.angle);
    playerinput.dir.y = sin(player.angle);

    //printf(" ANGLE: %f DIR: %f | %f\n", player.angle, playerinput.dir.x, playerinput.dir.y);

    // Calculate dx dy
    player.deltaPos.x = (playerinput.dir.x * playerinput.input.y) * PLAYER_SPEED * deltaTime;
    player.deltaPos.y = (playerinput.dir.y * playerinput.input.y) * PLAYER_SPEED * deltaTime;

    // Strafe
    if(playerinput.strafe.x != 0.0f)
    {
        float adjustedAngle = player.angle;
        vector2_t strafedDir;

        // Player wants to strafe
        if(playerinput.strafe.x >= 1.0f)
            adjustedAngle = player.angle + (M_PI / 2);
        else
            adjustedAngle = player.angle - (M_PI / 2);

        strafedDir.x = cos(adjustedAngle);
        strafedDir.y = sin(adjustedAngle);

        player.deltaPos.x += (strafedDir.x) * PLAYER_SPEED * deltaTime;
        player.deltaPos.y += (strafedDir.y) * PLAYER_SPEED * deltaTime;
    }

    // After calculating the movement, check for collision and in case, cancel the delta
    G_PlayerCollisionCheck();
    
    // Move Player normally
    player.position.x += player.deltaPos.x;
    player.position.y += player.deltaPos.y;

    // Clamp player in map boundaries
    player.position.x = SDL_clamp(player.position.x, 0.0f, (MAP_WIDTH * TILE_SIZE)-(TILE_SIZE/2));
    player.position.y = SDL_clamp(player.position.y, 0.0f, (MAP_WIDTH * TILE_SIZE)-(TILE_SIZE/2));

    // Compute centered pos for calculations
    player.centeredPos.x = player.position.x + PLAYER_CENTER_FIX;
    player.centeredPos.y = player.position.y + PLAYER_CENTER_FIX;

    // Update collision circle
    player.collisionCircle.pos.x = player.centeredPos.x;
    player.collisionCircle.pos.y = player.centeredPos.y;
}


void G_PlayerCollisionCheck()
{
    //---------------------------------------------------
    // Player->Collision Map
    //---------------------------------------------------

    // Calculate the player position relative to cell
    float playerXCellOffset = (int)(player.position.x+PLAYER_CENTER_FIX) % TILE_SIZE;
    float playerYCellOffset = (int)(player.position.y+PLAYER_CENTER_FIX) % TILE_SIZE;

    // Collision detection (Walls and solid sprites)
    if(player.deltaPos.x > 0)
    {
        int coll = G_CheckCollisionMap(player.level, player.gridPosition.y, player.gridPosition.x+1);

        // Player is moving right, check if it's too right
        if(coll != 0 && playerXCellOffset > (TILE_SIZE-PLAYER_MIN_DIST_TO_WALL)) // Wall check
            player.deltaPos.x = 0;
    }
    else
    {
        int coll  = G_CheckCollisionMap(player.level, player.gridPosition.y, player.gridPosition.x-1);

        // Player is moving left
        if(coll != 0 && playerXCellOffset < PLAYER_MIN_DIST_TO_WALL) // Wall check
            player.deltaPos.x = 0;
    }

    if(player.deltaPos.y < 0)
    {
        int coll  = G_CheckCollisionMap(player.level, player.gridPosition.y-1, player.gridPosition.x);

        // Player is going up
        if(coll != 0 && playerYCellOffset < PLAYER_MIN_DIST_TO_WALL) // Wall check
            player.deltaPos.y = 0;
    }
    else
    {
        int coll  = G_CheckCollisionMap(player.level, player.gridPosition.y+1, player.gridPosition.x);

        // Player is going down
        if(coll != 0 && playerYCellOffset > (TILE_SIZE-PLAYER_MIN_DIST_TO_WALL)) // Wall check
            player.deltaPos.y = 0;
    }

    //---------------------------------------------------
    // Player->Dynamic Sprites
    //---------------------------------------------------

    // The circle the player has if the delta movement is applied
    circle_t hypoteticalPlayerCircle = {player.collisionCircle.pos.x += player.deltaPos.x, player.collisionCircle.pos.y += player.deltaPos.y, player.collisionCircle.r};
    for(int i = 0; i < allDynamicSpritesLength; i++)
    {
        if(allDynamicSprites[i]->active && allDynamicSprites[i]->level == player.level)
        {
            sprite_t* cur = allDynamicSprites[i];

            // Check for collision
            // If there's collision, do not apply movement
            if(P_CheckCircleCollision(&hypoteticalPlayerCircle, &cur->collisionCircle) > 0)
            {
                player.deltaPos.x = 0;
                player.deltaPos.y = 0;
            }
        }
    }
}


//-------------------------------------
// Handles Input from the player 
//-------------------------------------
void G_InGameInputHandling(const uint8_t* keyboardState, SDL_Event* e)
{
    // Forward / backwards
    if(keyboardState[SDL_SCANCODE_UP] || keyboardState[SDL_SCANCODE_W])
        playerinput.input.y += 1.0f;
    else if(keyboardState[SDL_SCANCODE_DOWN] || keyboardState[SDL_SCANCODE_S])
        playerinput.input.y -= 1.0f;
    else
        playerinput.input.y = 0.0f;
    
    // Strafe right and left
    if(keyboardState[SDL_SCANCODE_A])
        playerinput.strafe.x = -1.0f; 
    else if(keyboardState[SDL_SCANCODE_D])
        playerinput.strafe.x = 1.0f; 
    else
        playerinput.strafe.x = 0.0f; 

    if(keyboardState[SDL_SCANCODE_LCTRL])
        if(player.z > 1)
            player.z -= 1.0f; 

    if(keyboardState[SDL_SCANCODE_LSHIFT])
        if(player.z < 191)
            player.z += 1.0f; 

    //playerinput.input.x = SDL_clamp(playerinput.input.x, -1.0f , 1.0f);
    playerinput.input.y = SDL_clamp(playerinput.input.y, -1.0f , 1.0f);
}


SDL_Rect fpRect = {0, 0, PROJECTION_PLANE_WIDTH, PROJECTION_PLANE_HEIGHT};
void G_PlayerRender(void)
{
    R_BlitIntoScreen(&fpRect, tomentdatapack.playersFP[PLAYER_FP_HANDS_IDLE]->texture, NULL);
}

//-------------------------------------
// Handles Input from the player while doing the Event Input Handling
//-------------------------------------
void G_InGameInputHandlingEvent(SDL_Event* e)
{
    switch(e->type)
    {
        case SDL_MOUSEMOTION:
            playerinput.input.x = e->motion.xrel;
            break;

        case SDL_KEYUP:
            // Space player's interacions
            if(e->key.keysym.sym == SDLK_SPACE)
            {
                // Interactions
                objectType_e objType = G_GetFromObjectTMap(player.level, player.inFrontGridPosition.y, player.inFrontGridPosition.x);

                if(objType == ObjT_Door)
                {
                    printf("Tapped a door\n");

                    // Open/Close
                    if(G_GetDoorState(player.level, player.inFrontGridPosition.y, player.inFrontGridPosition.x) == DState_Closed || G_GetDoorState(player.level,player.inFrontGridPosition.y, player.inFrontGridPosition.x) == DState_Closing)
                        G_SetDoorState(player.level, player.inFrontGridPosition.y, player.inFrontGridPosition.x, DState_Opening);
                    
                    else if(G_GetDoorState(player.level,player.inFrontGridPosition.y, player.inFrontGridPosition.x) == DState_Open || G_GetDoorState(player.level,player.inFrontGridPosition.y, player.inFrontGridPosition.x) == DState_Opening)
                        G_SetDoorState(player.level, player.inFrontGridPosition.y, player.inFrontGridPosition.x, DState_Closing);
                }
                else if(objType == ObjT_Empty)
                {
                    printf("Tapped an empty space\n");
                }
                else if(objType == ObjT_Sprite)
                {
                    printf("Tapped a sprite\n");
                }
                else if(objType == ObjT_Wall)
                {
                    printf("Tapped a wall\n");
                }
                else if(objType == ObjT_Trigger)
                {
                    printf("Tapped a trigger\n");

                    int wallID = R_GetValueFromLevel(player.level, player.inFrontGridPosition.y, player.inFrontGridPosition.x);
                    if(tomentdatapack.walls[wallID]->Callback != NULL)
                        tomentdatapack.walls[wallID]->Callback(tomentdatapack.walls[wallID]->data);
                }
            }

            if(e->key.keysym.sym == SDLK_ESCAPE)
            {
                A_ChangeState(GSTATE_MENU);
            }

            if(e->key.keysym.sym == SDLK_F1)
            {
                debugRendering = !debugRendering;
            }

        break;
    }
}

//-------------------------------------
// Checks the collision map at player's level and returns what found
//-------------------------------------
int G_CheckCollisionMap(int level, int y, int x)
{
    switch(level)
    {
        case 0:
            return currentMap.collisionMapLevel0[y][x];

        case 1:
            return currentMap.collisionMapLevel1[y][x];

        case 2:
            return currentMap.collisionMapLevel2[y][x];
    }
}

//-------------------------------------
// Checks door state map at player's level and returns what found
//-------------------------------------
int G_GetDoorState(int level, int y, int x)
{
    switch(level)
    {
        case 0:
            return doorstateLevel0[y][x];

        case 1:
            return doorstateLevel1[y][x];

        case 2:
            return doorstateLevel2[y][x];
    }
}

//-------------------------------------
// Checks door state map at player's level and returns what found
//-------------------------------------
int G_SetDoorState(int level, int y, int x, doorstate_e state)
{
    switch(level)
    {
        case 0:
            doorstateLevel0[y][x] = state;
            break;

        case 1:
            doorstateLevel1[y][x] = state;
            break;

        case 2:
            doorstateLevel2[y][x] = state;
            break;
    }
}

//-------------------------------------
// Checks door state map at player's level and returns what found
//-------------------------------------
float G_GetDoorPosition(int level, int y, int x)
{
    switch(level)
    {
        case 0:
            return doorpositionsLevel0[y][x];

        case 1:
            return doorpositionsLevel1[y][x];

        case 2:
            return doorpositionsLevel2[y][x];
    }
}

//-------------------------------------
// Checks object T map at player's level and returns what found
//-------------------------------------
int G_GetFromObjectTMap(int level, int y, int x)
{
    switch(level)
    {
        case 0:
            return currentMap.objectTMapLevel0[y][x];

        case 1:
            return currentMap.objectTMapLevel1[y][x];

        case 2:
            return currentMap.objectTMapLevel2[y][x];
    }
}