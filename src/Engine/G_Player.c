#include <math.h>

#include "G_Player.h"
#include "R_Rendering.h"
#include "I_InputHandling.h"
#include "M_Map.h"
#include "P_Physics.h"
#include "U_Utilities.h"
#include "G_Pathfinding.h"
#include "G_AI.h"

player_t player;    // Player

static void I_DetermineInFrontGrid(void);
static void I_SetAttackCone(int id, int x, int y);

static bool I_PlayerAttack(int attackType);
static bool I_PlayerCastSpell(int attackType);

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
    player.collisionCircle.r = TILE_SIZE / 2;

    // Init anim
    player.state = PSTATE_IDLE;
    player.animTimer = U_TimerCreateNew();
    player.animFrame = 0;
    player.animPlay = true;
    player.animPlayOnce = false;
    player.animTimer->Start(player.animTimer);

    // Init attributes
    player.attributes.maxHealth = 100.0f;
    player.attributes.curHealth = player.attributes.maxHealth;
      
    player.attributes.maxMana = 100.0f;
    player.attributes.curMana = player.attributes.maxMana;
    
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

    I_DetermineInFrontGrid();
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
        if(allDynamicSprites[i]->base.active && allDynamicSprites[i]->base.level == player.level)
        {
            dynamicSprite_t* cur = allDynamicSprites[i];

            // Check for collision
            // If there's collision, do not apply movement
            if(P_CheckCircleCollision(&hypoteticalPlayerCircle, &cur->base.collisionCircle) > 0)
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

    if(keyboardState[SDL_SCANCODE_KP_MINUS])
    {
        player.attributes.curHealth -= 1.0f;
    }

    if(keyboardState[SDL_SCANCODE_KP_PLUS])
    {
        player.attributes.curHealth += 1.0f;
    }

    if(keyboardState[SDL_SCANCODE_KP_3])
    {
        player.attributes.curMana -= 1.0f;
    }

    if(keyboardState[SDL_SCANCODE_KP_6])
    {
        player.attributes.curMana += 1.0f;
    }


    //playerinput.input.x = SDL_clamp(playerinput.input.x, -1.0f , 1.0f);
    playerinput.input.y = SDL_clamp(playerinput.input.y, -1.0f , 1.0f);
}


// FPS Images size are: PROJECTION_PLANE_WIDTH/2 by PROJECTION_PLANE_HEIGHT/2
void G_PlayerRender(void)
{
    SDL_Rect screenPos = {0, 0, PROJECTION_PLANE_WIDTH, PROJECTION_PLANE_HEIGHT};
    SDL_Rect size = {(0), (0), PROJECTION_PLANE_WIDTH/2, PROJECTION_PLANE_HEIGHT/2};

    // Select Animation
    SDL_Surface* curAnim;
    int curAnimLength = 0;

    switch(player.state)
    {
        case PSTATE_IDLE:
            curAnim = tomentdatapack.playersFP[PLAYER_FP_HANDS]->animations->animIdle;
            curAnimLength = tomentdatapack.playersFP[PLAYER_FP_HANDS]->animations->animIdleSheetLength;
            break;

        case PSTATE_ATTACKING1:
            curAnim = tomentdatapack.playersFP[PLAYER_FP_HANDS]->animations->animAttack;
            curAnimLength = tomentdatapack.playersFP[PLAYER_FP_HANDS]->animations->animAttackSheetLength;
            break;

        case PSTATE_CASTSPELL:
            curAnim = tomentdatapack.playersFP[PLAYER_FP_HANDS]->animations->animCastSpell;
            curAnimLength = tomentdatapack.playersFP[PLAYER_FP_HANDS]->animations->animCastSpellSheetLength;
            break;

        default:
            curAnim = tomentdatapack.playersFP[PLAYER_FP_HANDS]->animations->animIdle;
            curAnimLength = tomentdatapack.playersFP[PLAYER_FP_HANDS]->animations->animIdleSheetLength;
            break;
    }

    if(player.animPlay)
    {
        if(player.animPlayOnce)
        {
            if(curAnimLength > 0)
                player.animFrame = ((int)floor(player.animTimer->GetTicks(player.animTimer) / ANIMATION_SPEED_DIVIDER) % curAnimLength);

            // Spawn projectile at the right frame
            if(player.state == PSTATE_CASTSPELL && player.animFrame == 4 && player.hasToCast && !player.hasCasted)
            {
                // Spawn a projectile
                G_SpawnProjectile(S_Fireball1, player.angle, player.level, player.position.x + cos(player.angle) * TILE_SIZE, player.position.y + sin(player.angle) * TILE_SIZE, true);
                player.hasCasted = true;
                player.hasToCast = false;
            }

            // Prevent loop
            if(player.animFrame >= curAnimLength-1)
            {
                player.animPlay = false;
                
                // Go back to idle
                if(player.state == PSTATE_ATTACKING1 ||
                   player.state == PSTATE_CASTSPELL)
                   {
                        player.state = PSTATE_IDLE;
                        player.hasToCast = false;
                   }
            }
        }
        else
        {
            // Allow loop
             if(curAnimLength > 0)
                player.animFrame = ((int)floor(player.animTimer->GetTicks(player.animTimer) / ANIMATION_SPEED_DIVIDER) % curAnimLength);
        }

        size.x = (PROJECTION_PLANE_WIDTH/2) * player.animFrame; 
    }

    // Blit FP
    R_BlitIntoScreenScaled(&size, curAnim, &screenPos);
}

// -----------------------------------
// Renders Player's UI
// -----------------------------------
void G_PlayerUIRender(void)
{
    // HEALTH BAR
    SDL_Rect healthbarEmptyScreenPos = {105, 5, PROJECTION_PLANE_WIDTH, PROJECTION_PLANE_HEIGHT};
    SDL_Rect healthbarEmptySize = {(0), (0), PROJECTION_PLANE_WIDTH, PROJECTION_PLANE_HEIGHT};

    R_BlitIntoScreenScaled(&healthbarEmptySize, tomentdatapack.uiAssets[G_ASSET_HEALTHBAR_EMPTY].texture, &healthbarEmptyScreenPos);

    SDL_Rect healthbarFillScreenPos = {105, 5, PROJECTION_PLANE_WIDTH, PROJECTION_PLANE_HEIGHT};
    SDL_Rect healthbarFillSize = {(0), (0), PROJECTION_PLANE_WIDTH, PROJECTION_PLANE_HEIGHT};

    healthbarFillSize.x = player.attributes.maxHealth-player.attributes.curHealth;

    // Fix bar border
    if(healthbarFillSize.x > 3)
        healthbarFillScreenPos.x+=3;

    R_BlitIntoScreenScaled(&healthbarFillSize, tomentdatapack.uiAssets[G_ASSET_HEALTHBAR_FILL].texture, &healthbarFillScreenPos);

    // MANA BAR
    SDL_Rect manabarEmptyScreenPos = {105, 34, PROJECTION_PLANE_WIDTH, PROJECTION_PLANE_HEIGHT};
    SDL_Rect manabarEmptySize = {(0), (0), PROJECTION_PLANE_WIDTH, PROJECTION_PLANE_HEIGHT};

    R_BlitIntoScreenScaled(&manabarEmptySize, tomentdatapack.uiAssets[G_ASSET_MANABAR_EMPTY].texture, &manabarEmptyScreenPos);

    SDL_Rect manabarFillScreenPos = {105, 34, PROJECTION_PLANE_WIDTH, PROJECTION_PLANE_HEIGHT};
    SDL_Rect manabarFillSize = {(0), (0), PROJECTION_PLANE_WIDTH, PROJECTION_PLANE_HEIGHT};

    manabarFillSize.x = player.attributes.maxMana-player.attributes.curMana;

    // Fix bar border
    if(manabarFillSize.x > 3)
        manabarFillScreenPos.x+=3;

    R_BlitIntoScreenScaled(&manabarFillSize, tomentdatapack.uiAssets[G_ASSET_MANABAR_FILL].texture, &manabarFillScreenPos);
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

        case SDL_MOUSEBUTTONUP:
            if(e->button.button == SDL_BUTTON_LEFT)
            {
                if(G_PlayerCanAttack())
                    I_PlayerAttack(0);
            }


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

            if(e->key.keysym.sym == SDLK_c)
            {
                if(G_PlayerCanAttack())
                    I_PlayerCastSpell(0);
            }


            if(e->key.keysym.sym == SDLK_ESCAPE)
            {
                A_ChangeState(GSTATE_MENU);
            }

            if(e->key.keysym.sym == SDLK_F1)
            {
                debugRendering = !debugRendering;
            }

            if(e->key.keysym.sym == SDLK_F2)
            {
                r_debugPathfinding = true;

                // Update minimap
                R_DrawMinimap();

                if(allDynamicSprites[0] != NULL)
                    G_PerformPathfindingDebug(allDynamicSprites[0]->base.level, allDynamicSprites[0]->base.gridPos, player.gridPosition);
                else
                {
                    printf("Tried to debug pathfinding between first dynamic sprite and player, but no dynamic sprite is present in this map... Performing Pathfinding from center of the map to the player...\n");
                    vector2Int_t gridpos = {floor(MAP_WIDTH / 2), floor(MAP_HEIGHT / 2)};
                    G_PerformPathfindingDebug(player.level, gridpos, player.gridPosition);
                }

                r_debugPathfinding = false;
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

static void I_DetermineInFrontGrid(void)
{
    // Update front grid pos
    // Bottom cell
    if(player.angle < (2*M_PI)/3 && player.angle > M_PI/3)
    {
        player.inFrontGridPosition.x = player.gridPosition.x;
        player.inFrontGridPosition.y = player.gridPosition.y +1;

        // Set Attack Cone
        // Center Cell
        I_SetAttackCone(0, player.gridPosition.x, player.gridPosition.y);

        // In Front Cell
        I_SetAttackCone(1, player.inFrontGridPosition.x, player.inFrontGridPosition.y);

        // Right
        I_SetAttackCone(2, player.inFrontGridPosition.x+1, player.inFrontGridPosition.y);

        // Leeft 
        I_SetAttackCone(3, player.inFrontGridPosition.x-1, player.inFrontGridPosition.y);
        
    }
    // Top cell
    else if(player.angle > (4*M_PI)/3 && player.angle < (5*M_PI)/3)
    {
        player.inFrontGridPosition.x = player.gridPosition.x;
        player.inFrontGridPosition.y = player.gridPosition.y -1;

        // Set Attack Cone
        // Center Cell
        I_SetAttackCone(0, player.gridPosition.x, player.gridPosition.y);

        // In Front Cell
        I_SetAttackCone(1, player.inFrontGridPosition.x, player.inFrontGridPosition.y);

        // Right
        I_SetAttackCone(2, player.inFrontGridPosition.x+1, player.inFrontGridPosition.y);

        // Left 
        I_SetAttackCone(3, player.inFrontGridPosition.x-1, player.inFrontGridPosition.y);
    }
    // Right cell
    else if(player.angle < M_PI/6 || player.angle > (11*M_PI) / 6)
    {
        player.inFrontGridPosition.x = player.gridPosition.x +1;
        player.inFrontGridPosition.y = player.gridPosition.y;

        // Set Attack Cone
        // Center Cell
        I_SetAttackCone(0, player.gridPosition.x, player.gridPosition.y);

        // In Front Cell
        I_SetAttackCone(1, player.inFrontGridPosition.x, player.inFrontGridPosition.y);

        // Top
        I_SetAttackCone(2, player.inFrontGridPosition.x, player.inFrontGridPosition.y-1);

        // Bottom 
        I_SetAttackCone(3, player.inFrontGridPosition.x, player.inFrontGridPosition.y+1);
    }
    // Left Cell
    else if(player.angle > (5*M_PI)/6 && player.angle < (7*M_PI)/6)
    {
        player.inFrontGridPosition.x = player.gridPosition.x -1;
        player.inFrontGridPosition.y = player.gridPosition.y;

        // Set Attack Cone
        // Center Cell
        I_SetAttackCone(0, player.gridPosition.x, player.gridPosition.y);

        // In Front Cell
        I_SetAttackCone(1, player.inFrontGridPosition.x, player.inFrontGridPosition.y);

        // Top
        I_SetAttackCone(2, player.inFrontGridPosition.x, player.inFrontGridPosition.y-1);

        // Bottom 
        I_SetAttackCone(3, player.inFrontGridPosition.x, player.inFrontGridPosition.y+1);
    }
    // Top Right Cell
    else if(player.angle < (11*M_PI) / 6 && player.angle > (5*M_PI)/3)
    {
        player.inFrontGridPosition.x = player.gridPosition.x +1;
        player.inFrontGridPosition.y = player.gridPosition.y -1;

        // Set Attack Cone
        // Center Cell
        I_SetAttackCone(0, player.gridPosition.x, player.gridPosition.y);

        // In Front Cell
        I_SetAttackCone(1, player.inFrontGridPosition.x, player.inFrontGridPosition.y);

        // Left
        I_SetAttackCone(2, player.inFrontGridPosition.x-1, player.inFrontGridPosition.y);

        // Bottom 
        I_SetAttackCone(3, player.inFrontGridPosition.x, player.inFrontGridPosition.y+1);
    }
    // Top Left Cell
    else if(player.angle > (7*M_PI) / 6 && player.angle < (4*M_PI)/3)
    {
        player.inFrontGridPosition.x = player.gridPosition.x -1;
        player.inFrontGridPosition.y = player.gridPosition.y -1;

        // Set Attack Cone
        // Center Cell
        I_SetAttackCone(0, player.gridPosition.x, player.gridPosition.y);

        // In Front Cell
        I_SetAttackCone(1, player.inFrontGridPosition.x, player.inFrontGridPosition.y);

        // Right
        I_SetAttackCone(2, player.inFrontGridPosition.x+1, player.inFrontGridPosition.y);

        // Bottom 
        I_SetAttackCone(3, player.inFrontGridPosition.x, player.inFrontGridPosition.y+1);
    }
    // Bottom Right Cell
    else if(player.angle < M_PI/3 && player.angle > M_PI/6)
    {
        player.inFrontGridPosition.x = player.gridPosition.x +1;
        player.inFrontGridPosition.y = player.gridPosition.y +1;

        // Set Attack Cone
        // Center Cell
        I_SetAttackCone(0, player.gridPosition.x, player.gridPosition.y);

        // In Front Cell
        I_SetAttackCone(1, player.inFrontGridPosition.x, player.inFrontGridPosition.y);

        // Top
        I_SetAttackCone(2, player.inFrontGridPosition.x, player.inFrontGridPosition.y-1);

        // Left 
        I_SetAttackCone(3, player.inFrontGridPosition.x-1, player.inFrontGridPosition.y);
    }
    // Bottom Left Cell
    else if(player.angle > (2*M_PI)/3 && player.angle < (M_PI*5)/6)
    {
        player.inFrontGridPosition.x = player.gridPosition.x -1;
        player.inFrontGridPosition.y = player.gridPosition.y +1;

        // Set Attack Cone
        // Center Cell
        I_SetAttackCone(0, player.gridPosition.x, player.gridPosition.y);

        // In Front Cell
        I_SetAttackCone(1, player.inFrontGridPosition.x, player.inFrontGridPosition.y);

        // Top
        I_SetAttackCone(2, player.inFrontGridPosition.x, player.inFrontGridPosition.y-1);

        // Right 
        I_SetAttackCone(3, player.inFrontGridPosition.x+1, player.inFrontGridPosition.y);
    }
}

void G_PlayerPlayAnimationOnce(objectanimationsID_e animID)
{
    player.animTimer->Start(player.animTimer);
    player.animPlayOnce = true;
    player.animFrame = 0;

    switch(animID)
    {
        case ANIM_ATTACK1:
            player.state = PSTATE_ATTACKING1;
            break;

        case ANIM_CAST_SPELL:
            player.state = PSTATE_CASTSPELL;
            break;
    }

    player.animPlay = true;
}

static bool I_PlayerAttack(int attackType)
{
    G_PlayerPlayAnimationOnce(ANIM_ATTACK1);

    dynamicSprite_t* ai;

    for(int i = 0; i < ATTACK_CONE_SIZE; i++)
    {
        bool found;
        ai = G_GetFromDynamicSpriteMap(player.level, player.attackConeGridPos[i].y, player.attackConeGridPos[i].x);
        
        // We found one
        if(ai != NULL)
        {   
            // Check if AI is in front
            float anglediff = (int)((player.angle * (180/M_PI)) - ai->base.angle + 180 + 360) % 360 - 180;
            bool inFront = (anglediff <= ATTACK_CONE_MAX_DIFF && anglediff>=-ATTACK_CONE_MAX_DIFF);
        
            if(!inFront)
                ai= NULL;
            else
                break;
        }
    }

    if(ai != NULL && ai->base.dist < PLAYER_AI_HIT_DISTANCE)
    {
        printf("Hit an enemy.\n");
        G_AITakeDamage(ai, 10.0f);
        return true;
    }
    else
    {
        printf("Hit the air.\n");
        return false;
    }
}

static bool I_PlayerCastSpell(int attackType)
{
    G_PlayerPlayAnimationOnce(ANIM_CAST_SPELL);
    player.hasToCast = true;
    player.hasCasted = false;
}

static void I_SetAttackCone(int id, int x, int y)
{
    player.attackConeGridPos[id].x = x;
    player.attackConeGridPos[id].y = y;
}

bool G_PlayerCanAttack(void)
{
    return (player.state != PSTATE_ATTACKING1 && player.state != PSTATE_CASTSPELL);
}