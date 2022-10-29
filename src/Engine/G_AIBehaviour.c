#include "G_AIBehaviour.h"

#include <math.h>
#include "G_AI.h"
#include "G_Player.h"
#include "G_Pathfinding.h"
#include "P_Physics.h"
#include "G_AIBehaviour.h"

// Enemy-Specific Behaviours

void G_AI_BehaviourMeeleEnemy(dynamicSprite_t* cur)
{
    int oldGridPosX = cur->base.gridPos.x;
    int oldGridPosY = cur->base.gridPos.y;

    // Calculate centered pos
    cur->base.centeredPos.x = cur->base.pos.x + (HALF_TILE_SIZE);
    cur->base.centeredPos.y = cur->base.pos.y + (HALF_TILE_SIZE);

    // Calculate runtime stuff
    // Get Player Space pos
    cur->base.pSpacePos.x = cur->base.centeredPos.x - player.centeredPos.x;
    cur->base.pSpacePos.y = cur->base.centeredPos.y - player.centeredPos.y;

    cur->base.gridPos.x = cur->base.centeredPos.x / TILE_SIZE;
    cur->base.gridPos.y = cur->base.centeredPos.y / TILE_SIZE;

    // Determine AI's level
    cur->base.level = (int)floor(cur->base.z / TILE_SIZE);
    cur->base.level = SDL_clamp(cur->base.level, 0, MAX_N_LEVELS-1);

    if(cur->verticalMovementDelta > 0 || cur->verticalMovementDelta < 0)
        cur->base.z += cur->verticalMovementDelta * deltaTime;

    cur->base.angle = ((atan2(-cur->base.pSpacePos.y, cur->base.pSpacePos.x))* RADIAN_TO_DEGREE)*-1;
    FIX_ANGLES_DEGREES(cur->base.angle);
    
    // Set the target as the player
    cur->targetPos = &player.centeredPos;
    cur->targetGridPos = &player.gridPosition;
    cur->targetColl = &player.collisionCircle;

    // Calculate the distance to player
    cur->base.dist = sqrt(cur->base.pSpacePos.x*cur->base.pSpacePos.x + cur->base.pSpacePos.y*cur->base.pSpacePos.y);

    // Movements
    if(cur->isAlive && G_AICanAttack(cur))
    {
        path_t path = G_PerformPathfinding(cur->base.level, cur->base.gridPos, *(cur->targetGridPos), cur);
        cur->path = &path;

        float deltaX = 0.0f;
        float deltaY = 0.0f; 

        // Check if path is valid and if there's space to follow it
        if(path.isValid && path.nodesLength-1 >= 0 && path.nodes[path.nodesLength-1] != NULL &&
            G_CheckDynamicSpriteMap(cur->base.level, path.nodes[path.nodesLength-1]->gridPos.y, path.nodes[path.nodesLength-1]->gridPos.x) == false)
        {
            // From here on, the AI is chasing the player so it is safe to say that they're fighting
            if(player.hasBeenInitialized)
                cur->aggroedPlayer = true;

            // Check boss fight
            if(!player.isFightingBoss && cur->isBoss)
            {
                player.isFightingBoss = true;
                player.bossFighting = cur;
            }

            deltaX = (path.nodes[path.nodesLength-1]->gridPos.x * TILE_SIZE + (HALF_TILE_SIZE)) - cur->base.centeredPos.x;
            deltaY = (path.nodes[path.nodesLength-1]->gridPos.y * TILE_SIZE + (HALF_TILE_SIZE)) - cur->base.centeredPos.y;

            // Check if we're far away from the target
            if(P_CheckCircleCollision(&cur->base.collisionCircle, cur->targetColl) < 0 && 
                P_GetDistance((*cur->targetPos).x, (*cur->targetPos).y, cur->base.centeredPos.x + ((deltaX * cur->speed) * deltaTime), cur->base.centeredPos.y + ((deltaX * cur->speed) * deltaTime)) > AI_STOP_DISTANCE)
                {
                    cur->base.pos.x += (deltaX * cur->speed) * deltaTime;
                    cur->base.pos.y += (deltaY * cur->speed) * deltaTime; 

                    // Recalculate centered pos after delta move
                    cur->base.centeredPos.x = cur->base.pos.x + (HALF_TILE_SIZE);
                    cur->base.centeredPos.y = cur->base.pos.y + (HALF_TILE_SIZE);

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

        // Check Attack
        if(cur->base.dist < AI_MELEE_ATTACK_DISTANCE && cur->base.level == player.level)
        {
            // In range for attacking
            G_AIPlayAnimationOnce(cur, ANIM_ATTACK1);
            G_AIAttackPlayer(cur);
            cur->aggroedPlayer = true;
        }
    }

    // Select Animation & Play it
    switch(cur->state)
    {
        case DS_STATE_IDLE:
            cur->curAnim = tomentdatapack.sprites[cur->base.spriteID]->animations->animIdle;
            cur->curAnimLength = tomentdatapack.sprites[cur->base.spriteID]->animations->animIdleSheetLength;
            break;

        case DS_STATE_DEAD:
            cur->curAnim = tomentdatapack.sprites[cur->base.spriteID]->animations->animDie;
            cur->curAnimLength = tomentdatapack.sprites[cur->base.spriteID]->animations->animDieSheetLength;
            break;

        case DS_STATE_ATTACKING:
            cur->curAnim = tomentdatapack.sprites[cur->base.spriteID]->animations->animAttack;
            cur->curAnimLength = tomentdatapack.sprites[cur->base.spriteID]->animations->animAttackSheetLength;
            break;

        case DS_STATE_SPECIAL1:
            cur->curAnim = tomentdatapack.sprites[cur->base.spriteID]->animations->animSpecial1;
            cur->curAnimLength = tomentdatapack.sprites[cur->base.spriteID]->animations->animSpecial1SheetLength;
            break;

        default:
            cur->curAnim = tomentdatapack.sprites[cur->base.spriteID]->animations->animIdle;
            cur->curAnimLength = tomentdatapack.sprites[cur->base.spriteID]->animations->animIdleSheetLength;
            break;
    }

    if(cur->animPlay)
    {
        if(cur->animPlayOnce)
        {
            if(cur->curAnimLength > 0)
                cur->animFrame = ((int)floor(cur->animTimer->GetTicks(cur->animTimer) / cur->animSpeed) % cur->curAnimLength);

            // Prevent loop
            if(cur->animFrame >= cur->curAnimLength-1)
            {
                cur->animPlay = false;

                // Go back to idle if it was attacking
                if(cur->state == DS_STATE_ATTACKING  || cur->state == DS_STATE_SPECIAL1)
                {
                    G_AIPlayAnimationLoop(cur, ANIM_IDLE);
                }
            }
        }
        else
        {
            // Allow loop
            if(cur->curAnimLength > 0)
                cur->animFrame = ((int)floor(cur->animTimer->GetTicks(cur->animTimer) / cur->animSpeed) % cur->curAnimLength);
        }
    }
}

void G_AI_BehaviourCasterEnemy(dynamicSprite_t* cur)
{
    int oldGridPosX = cur->base.gridPos.x;
    int oldGridPosY = cur->base.gridPos.y;

    // Calculate centered pos
    cur->base.centeredPos.x = cur->base.pos.x + (HALF_TILE_SIZE);
    cur->base.centeredPos.y = cur->base.pos.y + (HALF_TILE_SIZE);

    // Calculate runtime stuff
    // Get Player Space pos
    cur->base.pSpacePos.x = cur->base.centeredPos.x - player.centeredPos.x;
    cur->base.pSpacePos.y = cur->base.centeredPos.y - player.centeredPos.y;

    cur->base.gridPos.x = cur->base.centeredPos.x / TILE_SIZE;
    cur->base.gridPos.y = cur->base.centeredPos.y / TILE_SIZE;

    // Determine AI's level
    cur->base.level = (int)floor(cur->base.z / TILE_SIZE);
    cur->base.level = SDL_clamp(cur->base.level, 0, MAX_N_LEVELS-1);

    if(cur->verticalMovementDelta > 0 || cur->verticalMovementDelta < 0)
        cur->base.z += cur->verticalMovementDelta * deltaTime;

    cur->base.angle = ((atan2(-cur->base.pSpacePos.y, cur->base.pSpacePos.x))* RADIAN_TO_DEGREE)*-1;
    FIX_ANGLES_DEGREES(cur->base.angle);
    
    // Set the target as the player
    cur->targetPos = &player.centeredPos;
    cur->targetGridPos = &player.gridPosition;
    cur->targetColl = &player.collisionCircle;

    // Calculate the distance to player
    cur->base.dist = sqrt(cur->base.pSpacePos.x*cur->base.pSpacePos.x + cur->base.pSpacePos.y*cur->base.pSpacePos.y);

    // Movements
    if(cur->isAlive && G_AICanAttack(cur))
    {
        path_t path = G_PerformPathfinding(cur->base.level, cur->base.gridPos, *(cur->targetGridPos), cur);
        cur->path = &path;

        float deltaX = 0.0f;
        float deltaY = 0.0f; 

        // Check if path is valid and if there's space to follow it
        if(path.isValid && path.nodesLength-1 >= 0 && path.nodes[path.nodesLength-1] != NULL &&
            G_CheckDynamicSpriteMap(cur->base.level, path.nodes[path.nodesLength-1]->gridPos.y, path.nodes[path.nodesLength-1]->gridPos.x) == false)
        {
            // From here on, the AI is chasing the player so it is safe to say that they're fighting

            // Check boss fight
            if(!player.isFightingBoss && cur->isBoss)
            {
                player.isFightingBoss = true;
                player.bossFighting = cur;
            }

            deltaX = (path.nodes[path.nodesLength-1]->gridPos.x * TILE_SIZE + (HALF_TILE_SIZE)) - cur->base.centeredPos.x;
            deltaY = (path.nodes[path.nodesLength-1]->gridPos.y * TILE_SIZE + (HALF_TILE_SIZE)) - cur->base.centeredPos.y;

            // Check if we're far away from the target
            if(P_CheckCircleCollision(&cur->base.collisionCircle, cur->targetColl) < 0 && 
                P_GetDistance((*cur->targetPos).x, (*cur->targetPos).y, cur->base.centeredPos.x + ((deltaX * cur->speed) * deltaTime), cur->base.centeredPos.y + ((deltaX * cur->speed) * deltaTime)) > AI_STOP_DISTANCE)
                {
                    cur->base.pos.x += (deltaX * cur->speed) * deltaTime;
                    cur->base.pos.y += (deltaY * cur->speed) * deltaTime; 

                    // Recalculate centered pos after delta move
                    cur->base.centeredPos.x = cur->base.pos.x + (HALF_TILE_SIZE);
                    cur->base.centeredPos.y = cur->base.pos.y + (HALF_TILE_SIZE);

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

        // Check Attack
        // If the player is at attack distance OR if he was in combat before but the AI can't reach him to close up the distance (example: casters on towers)
        if(cur->base.dist < AI_SPELL_ATTACK_DISTANCE || (cur->aggroedPlayer && !path.isValid))
        {
            // In range for attacking (casting spell)
            G_AIPlayAnimationOnce(cur, ANIM_ATTACK1);
            cur->aggroedPlayer = true;
        }
    }

    // Select Animation & Play it
    switch(cur->state)
    {
        case DS_STATE_IDLE:
            cur->curAnim = tomentdatapack.sprites[cur->base.spriteID]->animations->animIdle;
            cur->curAnimLength = tomentdatapack.sprites[cur->base.spriteID]->animations->animIdleSheetLength;
            break;

        case DS_STATE_DEAD:
            cur->curAnim = tomentdatapack.sprites[cur->base.spriteID]->animations->animDie;
            cur->curAnimLength = tomentdatapack.sprites[cur->base.spriteID]->animations->animDieSheetLength;
            break;

        case DS_STATE_ATTACKING:
            cur->curAnim = tomentdatapack.sprites[cur->base.spriteID]->animations->animAttack;
            cur->curAnimLength = tomentdatapack.sprites[cur->base.spriteID]->animations->animAttackSheetLength;
            break;

        default:
            cur->curAnim = tomentdatapack.sprites[cur->base.spriteID]->animations->animIdle;
            cur->curAnimLength = tomentdatapack.sprites[cur->base.spriteID]->animations->animIdleSheetLength;
            break;
    }

    if(cur->animPlay)
    {
        if(cur->animPlayOnce)
        {
            if(cur->curAnimLength > 0)
                cur->animFrame = ((int)floor(cur->animTimer->GetTicks(cur->animTimer) / cur->animSpeed) % cur->curAnimLength);

            // Prevent loop
            if(cur->animFrame >= cur->curAnimLength-1)
            {
                cur->animPlay = false;

                // Spawn the spell and go back to idle
                if(cur->state == DS_STATE_ATTACKING)
                {
                    // Attack chance, casters may fail spell
                    int attack      =  (rand() % (100)) + 1;

                    if(attack <= cur->attributes.attackChance)
                    {
                        float projAngle = cur->base.angle + 180;
                        FIX_ANGLES_DEGREES(projAngle);
                        G_SpawnProjectile(cur->spellInUse, (projAngle) * (M_PI / 180), cur->base.level, cur->base.centeredPos.x, cur->base.centeredPos.y, cur->base.z, player.z-(cur->base.z+HALF_TILE_SIZE), false, cur);
                    }

                    G_AIPlayAnimationLoop(cur, ANIM_IDLE);
                }
            }
        }
        else
        {
            // Allow loop
            if(cur->curAnimLength > 0)
                cur->animFrame = ((int)floor(cur->animTimer->GetTicks(cur->animTimer) / cur->animSpeed) % cur->curAnimLength);
        }
    }
}

void G_AI_BehaviourSkeletonLord(dynamicSprite_t* cur)
{
    int oldGridPosX = cur->base.gridPos.x;
    int oldGridPosY = cur->base.gridPos.y;

    // Calculate centered pos
    cur->base.centeredPos.x = cur->base.pos.x + (HALF_TILE_SIZE);
    cur->base.centeredPos.y = cur->base.pos.y + (HALF_TILE_SIZE);

    // Calculate runtime stuff
    // Get Player Space pos
    cur->base.pSpacePos.x = cur->base.centeredPos.x - player.centeredPos.x;
    cur->base.pSpacePos.y = cur->base.centeredPos.y - player.centeredPos.y;

    cur->base.gridPos.x = cur->base.centeredPos.x / TILE_SIZE;
    cur->base.gridPos.y = cur->base.centeredPos.y / TILE_SIZE;

    // Determine AI's level
    cur->base.level = (int)floor(cur->base.z / TILE_SIZE);
    cur->base.level = SDL_clamp(cur->base.level, 0, MAX_N_LEVELS-1);

    if(cur->verticalMovementDelta > 0 || cur->verticalMovementDelta < 0)
        cur->base.z += cur->verticalMovementDelta * deltaTime;

    cur->base.angle = ((atan2(-cur->base.pSpacePos.y, cur->base.pSpacePos.x))* RADIAN_TO_DEGREE)*-1;
    FIX_ANGLES_DEGREES(cur->base.angle);
    
    // Set the target as the player
    cur->targetPos = &player.centeredPos;
    cur->targetGridPos = &player.gridPosition;
    cur->targetColl = &player.collisionCircle;

    // Calculate the distance to player
    cur->base.dist = sqrt(cur->base.pSpacePos.x*cur->base.pSpacePos.x + cur->base.pSpacePos.y*cur->base.pSpacePos.y);

    // Movements
    if(cur->isAlive && G_AICanAttack(cur))
    {
        path_t path = G_PerformPathfinding(cur->base.level, cur->base.gridPos, *(cur->targetGridPos), cur);
        cur->path = &path;

        float deltaX = 0.0f;
        float deltaY = 0.0f; 

        // Check if path is valid and if there's space to follow it
        if(path.isValid && path.nodesLength-1 >= 0 && path.nodes[path.nodesLength-1] != NULL &&
            G_CheckDynamicSpriteMap(cur->base.level, path.nodes[path.nodesLength-1]->gridPos.y, path.nodes[path.nodesLength-1]->gridPos.x) == false)
        {
            // From here on, the AI is chasing the player so it is safe to say that they're fighting
            if(player.hasBeenInitialized && !cur->aggroedPlayer)
            {
                cur->aggroedPlayer = true;
                cur->cooldowns[0]->Start(cur->cooldowns[0]);
                cur->cooldowns[1]->Start(cur->cooldowns[1]);
            }

            // Check boss fight
            if(!player.isFightingBoss && cur->isBoss)
            {
                player.isFightingBoss = true;
                player.bossFighting = cur;
            }

            deltaX = (path.nodes[path.nodesLength-1]->gridPos.x * TILE_SIZE + (HALF_TILE_SIZE)) - cur->base.centeredPos.x;
            deltaY = (path.nodes[path.nodesLength-1]->gridPos.y * TILE_SIZE + (HALF_TILE_SIZE)) - cur->base.centeredPos.y;

            // Check if we're far away from the target
            if(P_CheckCircleCollision(&cur->base.collisionCircle, cur->targetColl) < 0 && 
                P_GetDistance((*cur->targetPos).x, (*cur->targetPos).y, cur->base.centeredPos.x + ((deltaX * cur->speed) * deltaTime), cur->base.centeredPos.y + ((deltaX * cur->speed) * deltaTime)) > AI_STOP_DISTANCE)
                {
                    cur->base.pos.x += (deltaX * cur->speed) * deltaTime;
                    cur->base.pos.y += (deltaY * cur->speed) * deltaTime; 

                    // Recalculate centered pos after delta move
                    cur->base.centeredPos.x = cur->base.pos.x + (HALF_TILE_SIZE);
                    cur->base.centeredPos.y = cur->base.pos.y + (HALF_TILE_SIZE);

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

        // For ten seconds, attack melee or ranged
        if(cur->cooldowns[0]->GetTicks(cur->cooldowns[0]) < 10000)
        {
            // Check Attack
            if(cur->base.dist < AI_MELEE_ATTACK_DISTANCE && cur->base.level == player.level)
            {
                // In range for attacking
                G_AIPlayAnimationOnce(cur, ANIM_ATTACK1);
                G_AIAttackPlayer(cur);
                cur->aggroedPlayer = true;
            }
            else if(cur->base.dist > AI_MELEE_ATTACK_DISTANCE || cur->base.level == player.level)
            {
                // Check fireball cooldown
                if(cur->cooldowns[1]->GetTicks(cur->cooldowns[1]) > 3000)
                {
                    // In range for attacking (casting spell)
                    G_AIPlayAnimationOnce(cur, ANIM_CAST_SPELL);
                    cur->aggroedPlayer = true;
                }
            }
        }
        else
        {
            // Do a spell
            int spell =  rand() % (2);
            
            // Hell
            if(spell == 0)
            {
                G_AIPlayAnimationLoop(cur, ANIM_SPECIAL1);
            }
            // Resurrection
            else
            {
                G_AIPlayAnimationLoop(cur, ANIM_SPECIAL2);
            }
        }
    }
    
    if(cur->isAlive && cur->state == DS_STATE_SPECIAL1)
    {
        // Spell: Hell

        // 1) Reach the center of the arena
        if(cur->base.gridPos.x != 11 || cur->base.gridPos.y != 9)
        {
            vector2Int_t targetPos = {11,9};
            path_t path = G_PerformPathfinding(cur->base.level, cur->base.gridPos, targetPos, cur);
            cur->path = &path;

            float deltaX = 0.0f;
            float deltaY = 0.0f; 

            // Check if path is valid and if there's space to follow it
            if(path.isValid && path.nodesLength-1 >= 0 && path.nodes[path.nodesLength-1] != NULL &&
                G_CheckDynamicSpriteMap(cur->base.level, path.nodes[path.nodesLength-1]->gridPos.y, path.nodes[path.nodesLength-1]->gridPos.x) == false)
            {
                deltaX = (path.nodes[path.nodesLength-1]->gridPos.x * TILE_SIZE + (HALF_TILE_SIZE)) - cur->base.centeredPos.x;
                deltaY = (path.nodes[path.nodesLength-1]->gridPos.y * TILE_SIZE + (HALF_TILE_SIZE)) - cur->base.centeredPos.y;

                // Check if we're far away from the target
                cur->base.pos.x += (deltaX * cur->speed) * deltaTime;
                cur->base.pos.y += (deltaY * cur->speed) * deltaTime; 

                // Recalculate centered pos after delta move
                cur->base.centeredPos.x = cur->base.pos.x + (HALF_TILE_SIZE);
                cur->base.centeredPos.y = cur->base.pos.y + (HALF_TILE_SIZE);

                cur->base.gridPos.x = cur->base.centeredPos.x / TILE_SIZE;
                cur->base.gridPos.y = cur->base.centeredPos.y / TILE_SIZE;


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
                    }
                }
                
                // Update collision circle
                cur->base.collisionCircle.pos.x = cur->base.centeredPos.x;
                cur->base.collisionCircle.pos.y = cur->base.centeredPos.y;
            }
        }
        // 2) Fly in the sky
        else if(cur->base.z < 128 && !cur->cooldowns[2]->isStarted)
        {
            // Fly
            cur->verticalMovementDelta = 100.0f;
        }
        // 3) Charge and go down
        else
        {
            // Hell cooldown
            if(!cur->cooldowns[2]->isStarted)
            {
                cur->cooldowns[2]->Start(cur->cooldowns[2]);
                cur->base.z = 128;
                cur->verticalMovementDelta = 0;
            }

            if(cur->cooldowns[2]->GetTicks(cur->cooldowns[2]) > 500)
            {
                if(cur->base.z > 0)
                {
                    cur->verticalMovementDelta = -400.0f;
                }
                else
                {
                    cur->verticalMovementDelta = 0;
                    cur->base.z = 0;

                    float projAngle = cur->base.angle + 180;
                    for(int i = 0; i < 36; i++)
                    {
                        FIX_ANGLES_DEGREES(projAngle);
                        G_SpawnProjectile(cur->spellInUse, (projAngle) * (M_PI / 180), cur->base.level, cur->base.centeredPos.x, cur->base.centeredPos.y, cur->base.z, player.z-(cur->base.z+HALF_TILE_SIZE), false, cur);
                        projAngle += 10;
                    }

                    cur->cooldowns[0]->Start(cur->cooldowns[0]);
                    cur->cooldowns[2]->Stop(cur->cooldowns[2]);
                    G_AIPlayAnimationLoop(cur, ANIM_IDLE);
                }
            }
        }
    }
    else if(cur->isAlive && cur->state == DS_STATE_SPECIAL2)
    {
        // Spell: Hell

        // 1) Reach the center of the arena
        if(cur->base.gridPos.x != 11 || cur->base.gridPos.y != 9)
        {
            vector2Int_t targetPos = {11,9};
            path_t path = G_PerformPathfinding(cur->base.level, cur->base.gridPos, targetPos, cur);
            cur->path = &path;

            float deltaX = 0.0f;
            float deltaY = 0.0f; 

            // Check if path is valid and if there's space to follow it
            if(path.isValid && path.nodesLength-1 >= 0 && path.nodes[path.nodesLength-1] != NULL &&
                G_CheckDynamicSpriteMap(cur->base.level, path.nodes[path.nodesLength-1]->gridPos.y, path.nodes[path.nodesLength-1]->gridPos.x) == false)
            {
                deltaX = (path.nodes[path.nodesLength-1]->gridPos.x * TILE_SIZE + (HALF_TILE_SIZE)) - cur->base.centeredPos.x;
                deltaY = (path.nodes[path.nodesLength-1]->gridPos.y * TILE_SIZE + (HALF_TILE_SIZE)) - cur->base.centeredPos.y;

                // Check if we're far away from the target
                cur->base.pos.x += (deltaX * cur->speed) * deltaTime;
                cur->base.pos.y += (deltaY * cur->speed) * deltaTime; 

                // Recalculate centered pos after delta move
                cur->base.centeredPos.x = cur->base.pos.x + (HALF_TILE_SIZE);
                cur->base.centeredPos.y = cur->base.pos.y + (HALF_TILE_SIZE);

                cur->base.gridPos.x = cur->base.centeredPos.x / TILE_SIZE;
                cur->base.gridPos.y = cur->base.centeredPos.y / TILE_SIZE;


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
                    }
                }
                
                // Update collision circle
                cur->base.collisionCircle.pos.x = cur->base.centeredPos.x;
                cur->base.collisionCircle.pos.y = cur->base.centeredPos.y;
            }   
        }
        // 2) Fly in the sky
        else if(cur->base.z < 128 && !cur->cooldowns[3]->isStarted)
        {
            // Fly
            cur->verticalMovementDelta = 100.0f;
        }
        // 3) Resurrect and go down
        else
        {
            // Resurrect cooldown
            if(!cur->cooldowns[3]->isStarted)
            {
                cur->cooldowns[3]->Start(cur->cooldowns[3]);
                cur->base.z = 128;
                cur->verticalMovementDelta = 0;

                // Edge case, if the boss spawns over 255 minions, free up a bit of the array to allow the new to be spawned
                if(allDynamicSpritesLength+4 >= OBJECTARRAY_DEFAULT_SIZE)
                {
                    for(int i = OBJECTARRAY_DEFAULT_SIZE-5; i < OBJECTARRAY_DEFAULT_SIZE; i++)
                    {
                        if(allDynamicSprites[i] != NULL)
                            free(allDynamicSprites[i]);
                    }

                    allDynamicSpritesLength = OBJECTARRAY_DEFAULT_SIZE-5;
                }

                // Spawn AI
                if(currentMap.dynamicSpritesLevel0[cur->base.gridPos.y][cur->base.gridPos.x+1] == NULL)
                {
                    currentMap.dynamicSpritesLevel0[cur->base.gridPos.y][cur->base.gridPos.x+1] = (dynamicSprite_t*)malloc(sizeof(dynamicSprite_t));
                    dynamicSprite_t* spawned = currentMap.dynamicSpritesLevel0[cur->base.gridPos.y][cur->base.gridPos.x+1];
                    G_AIInitialize(spawned, 0, 3, cur->base.gridPos.x+1, cur->base.gridPos.y);
                    G_AIPlayAnimationOnce(spawned, ANIM_SPECIAL1);
                }

                // Spawn AI
                if(currentMap.dynamicSpritesLevel0[cur->base.gridPos.y][cur->base.gridPos.x-1] == NULL)
                {
                    currentMap.dynamicSpritesLevel0[cur->base.gridPos.y][cur->base.gridPos.x-1] = (dynamicSprite_t*)malloc(sizeof(dynamicSprite_t));
                    dynamicSprite_t* spawned = currentMap.dynamicSpritesLevel0[cur->base.gridPos.y][cur->base.gridPos.x-1];
                    G_AIInitialize(spawned, 0, 3, cur->base.gridPos.x-1, cur->base.gridPos.y);
                    G_AIPlayAnimationOnce(spawned, ANIM_SPECIAL1);
                }

                // Spawn AI
                if(currentMap.dynamicSpritesLevel0[cur->base.gridPos.y+1][cur->base.gridPos.x] == NULL)
                {
                    currentMap.dynamicSpritesLevel0[cur->base.gridPos.y+1][cur->base.gridPos.x] = (dynamicSprite_t*)malloc(sizeof(dynamicSprite_t));
                    dynamicSprite_t* spawned = currentMap.dynamicSpritesLevel0[cur->base.gridPos.y+1][cur->base.gridPos.x];
                    G_AIInitialize(spawned, 0, 3, cur->base.gridPos.x, cur->base.gridPos.y+1);
                    G_AIPlayAnimationOnce(spawned, ANIM_SPECIAL1);
                }

                // Spawn AI
                if(currentMap.dynamicSpritesLevel0[cur->base.gridPos.y-1][cur->base.gridPos.x] == NULL)
                {
                    currentMap.dynamicSpritesLevel0[cur->base.gridPos.y-1][cur->base.gridPos.x] = (dynamicSprite_t*)malloc(sizeof(dynamicSprite_t));
                    dynamicSprite_t* spawned = currentMap.dynamicSpritesLevel0[cur->base.gridPos.y-1][cur->base.gridPos.x];
                    G_AIInitialize(spawned, 0, 3, cur->base.gridPos.x, cur->base.gridPos.y-1);
                    G_AIPlayAnimationOnce(spawned, ANIM_SPECIAL1);
                }
                
            }

            if(cur->cooldowns[3]->GetTicks(cur->cooldowns[3]) > 2000)
            {
                if(cur->base.z > 0)
                {
                    cur->verticalMovementDelta = -400.0f;
                }
                else
                {
                    cur->verticalMovementDelta = 0;
                    cur->base.z = 0;

                    cur->cooldowns[0]->Start(cur->cooldowns[0]);
                    cur->cooldowns[3]->Stop(cur->cooldowns[3]);
                    G_AIPlayAnimationLoop(cur, ANIM_IDLE);
                }
            }
        }
    }


    // Select Animation & Play it
    switch(cur->state)
    {
        case DS_STATE_IDLE:
            cur->curAnim = tomentdatapack.sprites[cur->base.spriteID]->animations->animIdle;
            cur->curAnimLength = tomentdatapack.sprites[cur->base.spriteID]->animations->animIdleSheetLength;
            break;

        case DS_STATE_DEAD:
            cur->curAnim = tomentdatapack.sprites[cur->base.spriteID]->animations->animDie;
            cur->curAnimLength = tomentdatapack.sprites[cur->base.spriteID]->animations->animDieSheetLength;
            break;

        case DS_STATE_ATTACKING:
            cur->curAnim = tomentdatapack.sprites[cur->base.spriteID]->animations->animAttack;
            cur->curAnimLength = tomentdatapack.sprites[cur->base.spriteID]->animations->animAttackSheetLength;
            break;

        case DS_STATE_CASTING:
            cur->curAnim = tomentdatapack.sprites[cur->base.spriteID]->animations->animCastSpell;
            cur->curAnimLength = tomentdatapack.sprites[cur->base.spriteID]->animations->animCastSpellSheetLength;
            break;

        case DS_STATE_SPECIAL1:
            cur->curAnim = tomentdatapack.sprites[cur->base.spriteID]->animations->animSpecial1;
            cur->curAnimLength = tomentdatapack.sprites[cur->base.spriteID]->animations->animSpecial1SheetLength;
            break;

        case DS_STATE_SPECIAL2:
            cur->curAnim = tomentdatapack.sprites[cur->base.spriteID]->animations->animSpecial2;
            cur->curAnimLength = tomentdatapack.sprites[cur->base.spriteID]->animations->animSpecial2SheetLength;
            break;

        default:
            cur->curAnim = tomentdatapack.sprites[cur->base.spriteID]->animations->animIdle;
            cur->curAnimLength = tomentdatapack.sprites[cur->base.spriteID]->animations->animIdleSheetLength;
            break;
    }

    if(cur->animPlay)
    {
        if(cur->animPlayOnce)
        {
            if(cur->curAnimLength > 0)
                cur->animFrame = ((int)floor(cur->animTimer->GetTicks(cur->animTimer) / cur->animSpeed) % cur->curAnimLength);

            // Prevent loop
            if(cur->animFrame >= cur->curAnimLength-1)
            {
                cur->animPlay = false;

                // Go back to idle if it was attacking
                if(cur->state == DS_STATE_ATTACKING)
                {
                    G_AIPlayAnimationLoop(cur, ANIM_IDLE);
                }
                else if(cur->state == DS_STATE_CASTING)
                {
                    // Spawn the spell and go back to idle
                    // Attack chance, casters may fail spell
                    int attack      =  (rand() % (100)) + 1;

                    if(attack <= cur->attributes.attackChance)
                    {
                        float projAngle = cur->base.angle + 180;
                        FIX_ANGLES_DEGREES(projAngle);
                        G_SpawnProjectile(cur->spellInUse, (projAngle) * (M_PI / 180), cur->base.level, cur->base.centeredPos.x, cur->base.centeredPos.y, cur->base.z, player.z-(cur->base.z+HALF_TILE_SIZE), false, cur);
                        projAngle += 10;
                        FIX_ANGLES_DEGREES(projAngle);
                        G_SpawnProjectile(cur->spellInUse, (projAngle) * (M_PI / 180), cur->base.level, cur->base.centeredPos.x, cur->base.centeredPos.y, cur->base.z, player.z-(cur->base.z+HALF_TILE_SIZE), false, cur);
                        projAngle -= 20;
                        FIX_ANGLES_DEGREES(projAngle);
                        G_SpawnProjectile(cur->spellInUse, (projAngle) * (M_PI / 180), cur->base.level, cur->base.centeredPos.x, cur->base.centeredPos.y, cur->base.z, player.z-(cur->base.z+HALF_TILE_SIZE), false, cur);

                    }
                    cur->cooldowns[1]->Start(cur->cooldowns[1]);
                    G_AIPlayAnimationLoop(cur, ANIM_IDLE);
                }
            }
        }
        else
        {
            // Allow loop
            if(cur->curAnimLength > 0)
                cur->animFrame = ((int)floor(cur->animTimer->GetTicks(cur->animTimer) / cur->animSpeed) % cur->curAnimLength);
        }
    }

    // Extra, close door of player's spawn
    if(doorstateLevel0[18][11] == DState_Open && player.gridPosition.y < 17 && cur->base.gridPos.y < 17)
    {
        G_SetDoorState(0, 18, 11, DState_Closing);
    }
}