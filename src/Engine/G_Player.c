#include <math.h>

#include "G_Player.h"
#include "R_Rendering.h"
#include "I_InputHandling.h"
#include "M_Map.h"
#include "G_Physics.h"
#include "U_Utilities.h"
#include "D_AssetsManager.h"

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
    player.position.x = PLAYER_STARTING_X;
    player.position.y = PLAYER_STARTING_Y;
    player.angle = 0.0f;

    player.gridPosition.x = PLAYER_STARTING_GRID_X;
    player.gridPosition.y = PLAYER_STARTING_GRID_Y;

    // Rect for minimap
    SDL_Rect_Set(&player.surfaceRect, (int)player.position.x, (int)player.position.y, PLAYER_WIDTH, PLAYER_HEIGHT);
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
    player.angle += playerinput.input.x * PLAYER_ROT_SPEED * deltaTime;
    
    FIX_ANGLES(player.angle);

    playerinput.dir.x = cos(player.angle);
    playerinput.dir.y = sin(player.angle);

    //printf(" ANGLE: %f DIR: %f | %f\n", player.angle, playerinput.dir.x, playerinput.dir.y);

    // Calculate dx dy
    player.deltaPos.x = (playerinput.dir.x * playerinput.input.y) * PLAYER_SPEED * deltaTime;
    player.deltaPos.y = (playerinput.dir.y * playerinput.input.y) * PLAYER_SPEED * deltaTime;

    // Calculate the player position relative to cell
    float playerXCellOffset = (int)(player.position.x+PLAYER_CENTER_FIX) % TILE_SIZE;
    float playerYCellOffset = (int)(player.position.y+PLAYER_CENTER_FIX) % TILE_SIZE;

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

    // Collision detection (Walls and solid sprites)
    if(player.deltaPos.x > 0)
    {
        int coll = currentMap.collisionMap[player.gridPosition.y][player.gridPosition.x+1];

        // Player is moving right, check if it's too right
        if(coll != 0 && playerXCellOffset > (TILE_SIZE-PLAYER_MIN_DIST_TO_WALL)) // Wall check
            player.deltaPos.x = 0;
    }
    else
    {
        int coll  = currentMap.collisionMap[player.gridPosition.y][player.gridPosition.x-1];

        // Player is moving left
        if(coll != 0 && playerXCellOffset < PLAYER_MIN_DIST_TO_WALL) // Wall check
            player.deltaPos.x = 0;
    }

    if(player.deltaPos.y < 0)
    {
        int coll  = currentMap.collisionMap[player.gridPosition.y-1][player.gridPosition.x];

        // Player is going up
        if(coll != 0 && playerYCellOffset < PLAYER_MIN_DIST_TO_WALL) // Wall check
            player.deltaPos.y = 0;
    }
    else
    {
        int coll  = currentMap.collisionMap[player.gridPosition.y+1][player.gridPosition.x];

        // Player is going down
        if(coll != 0 && playerYCellOffset > (TILE_SIZE-PLAYER_MIN_DIST_TO_WALL)) // Wall check
            player.deltaPos.y = 0;
    }
    
    // Move Player normally
    player.position.x += player.deltaPos.x;
    player.position.y += player.deltaPos.y;

    // Compute centered pos for calculations
    player.centeredPos.x = player.position.x + PLAYER_CENTER_FIX;
    player.centeredPos.y = player.position.y + PLAYER_CENTER_FIX;
}

//-------------------------------------
// Handles Input from the player 
//-------------------------------------
void G_PlayerHandleInput(const uint8_t* keyboardState, SDL_Event* e)
{
    // Left And right turn
    if(keyboardState[SDL_SCANCODE_LEFT])
        playerinput.input.x -= 1.0f; 
    else if(keyboardState[SDL_SCANCODE_RIGHT])
        playerinput.input.x += 1.0f; 
    else
        playerinput.input.x = 0.0f; 

    // Forward / backwards
    if(keyboardState[SDL_SCANCODE_UP])
        playerinput.input.y += 1.0f;
    else if(keyboardState[SDL_SCANCODE_DOWN])
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

    playerinput.input.x = SDL_clamp(playerinput.input.x, -1.0f , 1.0f);
    playerinput.input.y = SDL_clamp(playerinput.input.y, -1.0f , 1.0f);
}

//-------------------------------------
// Handles Input from the player while doing the Event Input Handling
//-------------------------------------
void G_PlayerHandleInputEvent(SDL_Event* e)
{
    switch(e->type)
    {
        case SDL_KEYUP:
            // Space player's interacions
            if(e->key.keysym.sym == SDLK_SPACE)
            {
                // Interactions
                objectType_e objType = currentMap.objectTMap[player.inFrontGridPosition.y][player.inFrontGridPosition.x];

                if(objType == ObjT_Door)
                {
                    printf("Tapped a door\n");

                    // Open/Close
                    if(doorstate[player.inFrontGridPosition.y][player.inFrontGridPosition.x] == DState_Closed || doorstate[player.inFrontGridPosition.y][player.inFrontGridPosition.x] == DState_Closing)
                        doorstate[player.inFrontGridPosition.y][player.inFrontGridPosition.x] = DState_Opening;
                    
                    else if(doorstate[player.inFrontGridPosition.y][player.inFrontGridPosition.x] == DState_Open || doorstate[player.inFrontGridPosition.y][player.inFrontGridPosition.x] == DState_Opening)
                        doorstate[player.inFrontGridPosition.y][player.inFrontGridPosition.x] = DState_Closing;
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
            }

        break;
    }
}