#include <math.h>

#include "G_AI.h"
#include "G_Player.h"
#include "G_Pathfinding.h"
#include "P_Physics.h"

// Dynamic AI list
dynamicSprite_t* allDynamicSprites[OBJECTARRAY_DEFAULT_SIZE];
unsigned int allDynamicSpritesLength = 0;

// Initializes the dynamic sprites
// Switch on the spriteID to be able to configure parameters for each entity, could be done in a file to avoid having to recompile to change stuff like speed
void G_AIInitialize(dynamicSprite_t* cur, int level, int spriteID, int x, int y)
{
      //---------------------
      // Base
      //---------------------
      cur->type = DS_TYPE_AI;
      
      cur->base.active = true;
      cur->base.level = level;

      cur->base.gridPos.x = x;
      cur->base.gridPos.y = y;

      // Get World Pos
      cur->base.pos.x = x * TILE_SIZE;
      cur->base.pos.y = y * TILE_SIZE;

      cur->base.collisionCircle.pos.x = cur->base.pos.x;
      cur->base.collisionCircle.pos.y = cur->base.pos.y;
      cur->base.collisionCircle.r = 32;

      // Get ID
      cur->base.spriteID = spriteID;
      cur->base.sheetLength = tomentdatapack.spritesSheetsLenghtTable[spriteID];

      //---------------------
      // Dynamic-Specific
      //---------------------
      cur->state = DS_STATE_NULL;
      cur->isAlive = true;
      cur->speed = 2.0f;
      
      cur->animTimer = U_TimerCreateNew();
      cur->animPlay = false;
      cur->animFrame = 0;
      cur->animPlayOnce = false;

      cur->attributes.maxhealth = 100.0f;
      cur->attributes.curHealth = cur->attributes.maxhealth;

      // Add it to the dynamic sprite list
      allDynamicSprites[allDynamicSpritesLength] = cur;
      allDynamicSpritesLength++;
}

void G_AIUpdate(void)
{
    for(int i = 0; i < allDynamicSpritesLength; i++)
    {
        dynamicSprite_t* cur = allDynamicSprites[i];

        if(cur == NULL || cur->base.active == false)
            continue;

        int oldGridPosX = cur->base.gridPos.x;
        int oldGridPosY = cur->base.gridPos.y;

        // Calculate centered pos
        cur->base.centeredPos.x = cur->base.pos.x + (TILE_SIZE / 2);
        cur->base.centeredPos.y = cur->base.pos.y + (TILE_SIZE / 2);

        // Calculate runtime stuff
        // Get Player Space pos
        cur->base.pSpacePos.x = cur->base.centeredPos.x - player.centeredPos.x;
        cur->base.pSpacePos.y = cur->base.centeredPos.y - player.centeredPos.y;

        cur->base.gridPos.x = cur->base.centeredPos.x / TILE_SIZE;
        cur->base.gridPos.y = cur->base.centeredPos.y / TILE_SIZE;

        cur->base.angle = ((atan2(-cur->base.pSpacePos.y, cur->base.pSpacePos.x))* RADIAN_TO_DEGREE)*-1;
        FIX_ANGLES_DEGREES(cur->base.angle);
        
        // Set the target as the player
        cur->targetPos = &player.centeredPos;
        cur->targetGridPos = &player.gridPosition;
        cur->targetColl = &player.collisionCircle;

        // Calculate the distance to player
        cur->base.dist = sqrt(cur->base.pSpacePos.x*cur->base.pSpacePos.x + cur->base.pSpacePos.y*cur->base.pSpacePos.y);

        if(cur->isAlive)
        {
            path_t path = G_PerformPathfinding(cur->base.level, cur->base.gridPos, *(cur->targetGridPos), cur);
            cur->path = &path;

            float deltaX = 0.0f;
            float deltaY = 0.0f; 

            // Check if path is valid and if there's space to follow it
            if(path.isValid && path.nodesLength-1 > 0 && path.nodes[path.nodesLength-1] != NULL &&
                G_CheckDynamicSpriteMap(cur->base.level, path.nodes[path.nodesLength-1]->gridPos.y, path.nodes[path.nodesLength-1]->gridPos.x) == false)
            {
                deltaX = (path.nodes[path.nodesLength-1]->gridPos.x * TILE_SIZE + (TILE_SIZE/2)) - cur->base.centeredPos.x;
                deltaY = (path.nodes[path.nodesLength-1]->gridPos.y * TILE_SIZE + (TILE_SIZE/2)) - cur->base.centeredPos.y;

                // Check if we're far away from the target
                if(P_CheckCircleCollision(&cur->base.collisionCircle, cur->targetColl) < 0 && 
                    P_GetDistance((*cur->targetPos).x, (*cur->targetPos).y, cur->base.centeredPos.x + ((deltaX * cur->speed) * deltaTime), cur->base.centeredPos.y + ((deltaX * cur->speed) * deltaTime)) > TILE_SIZE)
                    {
                        cur->base.pos.x += (deltaX * cur->speed) * deltaTime;
                        cur->base.pos.y += (deltaY * cur->speed) * deltaTime; 

                        // Recalculate centered pos after delta move
                        cur->base.centeredPos.x = cur->base.pos.x + (TILE_SIZE / 2);
                        cur->base.centeredPos.y = cur->base.pos.y + (TILE_SIZE / 2);

                        cur->base.gridPos.x = cur->base.centeredPos.x / TILE_SIZE;
                        cur->base.gridPos.y = cur->base.centeredPos.y / TILE_SIZE;

                        cur->state = DS_STATE_MOVING;
                    }
                    else
                    {
                        // Attacking
                        cur->state = DS_STATE_IDLE;
                    }
            }
            else
            {
                cur->state = DS_STATE_IDLE;
            }


            // Check if this AI changed grid pos
            if(!(oldGridPosX == cur->base.gridPos.x && oldGridPosY == cur->base.gridPos.y))
            {
                // If the tile the AI ended up in is not occupied

                if(G_CheckDynamicSpriteMap(cur->base.level, cur->base.gridPos.y, cur->base.gridPos.x) == false)
                {
                    // Update the dynamic map
                    switch(cur->base.level)
                    {
                        case 0:
                            currentMap.dynamicSpritesLevel0[cur->base.gridPos.y][cur->base.gridPos.x] = currentMap.dynamicSpritesLevel0[oldGridPosY][oldGridPosX];
                            currentMap.dynamicSpritesLevel0[oldGridPosY][oldGridPosX] = NULL;
                            break;
                        
                        case 1:
                            currentMap.dynamicSpritesLevel1[cur->base.gridPos.y][cur->base.gridPos.x] = currentMap.dynamicSpritesLevel1[oldGridPosY][oldGridPosX];
                            currentMap.dynamicSpritesLevel1[oldGridPosY][oldGridPosX] = NULL;
                            break;

                        case 2:
                            currentMap.dynamicSpritesLevel2[cur->base.gridPos.y][cur->base.gridPos.x] = currentMap.dynamicSpritesLevel2[oldGridPosY][oldGridPosX];
                            currentMap.dynamicSpritesLevel2[oldGridPosY][oldGridPosX] = NULL;
                            break;

                        default:
                        break;
                    }
                }
                else
                {
                    // Move back
                    cur->base.pos.x -= (deltaX * cur->speed) * deltaTime;
                    cur->base.pos.y -= (deltaY * cur->speed) * deltaTime; 

                    cur->base.gridPos.x = oldGridPosX;
                    cur->base.gridPos.y = oldGridPosY;

                    cur->state = DS_STATE_MOVING;
                }
            }
            
            // Update collision circle
            cur->base.collisionCircle.pos.x = cur->base.centeredPos.x;
            cur->base.collisionCircle.pos.y = cur->base.centeredPos.y;
        }
    }
}

void G_AIDie(dynamicSprite_t* cur)
{
    if(!cur->isAlive)
        return;

    cur->isAlive = false;
    G_AIPlayAnimationOnce(cur, ANIM_DIE);
    cur->base.collisionCircle.r = -1.0f; // Remove collision

    // Add to death map
    G_AddToDeadDynamicSpriteMap(cur, cur->base.level, cur->base.gridPos.y, cur->base.gridPos.x);

    // Clear from dynamic map
    G_ClearFromDynamicSpriteMap(cur->base.level, cur->base.gridPos.y, cur->base.gridPos.x);
}

void G_AIPlayAnimationOnce(dynamicSprite_t* cur, objectanimationsID_e animID)
{
    cur->animTimer->Start(cur->animTimer);
    cur->animPlayOnce = true;
    cur->animFrame = 0;

    switch(animID)
    {
        case ANIM_IDLE:
            cur->state = DS_STATE_IDLE;
            break;

        case ANIM_DIE:
            cur->state = DS_STATE_DEAD;
            break;
    }

    cur->animPlay = true;
}

void G_AIPlayAnimationLoop(dynamicSprite_t* cur, objectanimationsID_e animID)
{
    cur->animTimer->Start(cur->animTimer);
    cur->animPlayOnce = false;
    cur->animFrame = 0;

    switch(animID)
    {
        case ANIM_IDLE:
            cur->state = DS_STATE_IDLE;
            break;

        case ANIM_DIE:
            cur->state = DS_STATE_DEAD;
            break;
    }

    cur->animPlay = true;
}

void G_AITakeDamage(dynamicSprite_t* cur, float amount)
{
    if(cur != NULL && cur->isAlive)
    {
        cur->attributes.curHealth -= amount;
        
        // Check death
        if(cur->attributes.curHealth <= 0.0f)
        {
            G_AIDie(cur);
        }
    }
}