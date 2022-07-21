#include <math.h>

#include "G_Player.h"
#include "R_Rendering.h"
#include "I_InputHandling.h"
#include "M_Map.h"
#include "G_Physics.h"

player_t player; // Player
SDL_Rect destRect;

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
    player.surface = SDL_LoadBMP("Data/player.bmp");

    player.position.x = 12 * TILE_SIZE;
    player.position.y = 12 * TILE_SIZE;
    player.angle = 0.0f;

    SDL_Rect_Set(&player.surfaceRect, (int)player.position.x, (int)player.position.y, PLAYER_WIDTH, PLAYER_HEIGHT);

    player.gridPosition.x = 12.0f;
    player.gridPosition.y = 12.0f;

    player.projectionPlane.x = 0;
    player.projectionPlane.y = 0.66f;
}

//-------------------------------------
// Player's Tick 
//-------------------------------------
void G_PlayerTick(void)
{
    // Get player grid pos
    player.gridPosition.x = (player.position.x / TILE_SIZE);
    player.gridPosition.y = (player.position.y / TILE_SIZE);
    
    //player.angle = M_PI / 4;
    player.angle += playerinput.input.x * PLAYER_ROT_SPEED * deltaTime;

    
    if(player.angle > 2*M_PI)
        player.angle -= 2*M_PI;

    if(player.angle < 0)
        player.angle += 2*M_PI;
    

    playerinput.dir.x = cos(player.angle);
    playerinput.dir.y = sin(player.angle);

    //printf(" ANGLE: %f DIR: %f | %f\n", player.angle, playerinput.dir.x, playerinput.dir.y);

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

        player.position.x += (strafedDir.x) * PLAYER_SPEED * deltaTime;
        player.position.y += (strafedDir.y) * PLAYER_SPEED * deltaTime;
    }
    
    // Move Player normally
    player.position.x += (playerinput.dir.x * playerinput.input.y) * PLAYER_SPEED * deltaTime;
    player.position.y += (playerinput.dir.y * playerinput.input.y) * PLAYER_SPEED * deltaTime;

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
