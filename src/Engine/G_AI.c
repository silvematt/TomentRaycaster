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
    cur->base.level = SDL_clamp(cur->base.level, 0, MAX_N_LEVELS-1);

    cur->base.z = TILE_SIZE * (level);
    cur->verticalMovementDelta = 0.0f;
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
    cur->state = DS_STATE_IDLE;
    cur->isAlive = true;
    cur->speed = 2.0f;

    cur->curAnim = NULL;
    cur->curAnimLength = 0;
    cur->animTimer = U_TimerCreateNew();
    cur->animPlay = false;
    cur->animFrame = 0;
    cur->animPlayOnce = false;
    
    // Init attributes based on the sprite
    switch(cur->base.spriteID)
    {
        case DS_Skeleton:
            cur->base.name = "Skeleton";
            cur->isBoss = false;

            cur->BehaviourUpdate = G_AI_BehaviourMeeleEnemy;

            cur->speed = 2.0f;
            cur->attributes.maxHealth = 100.0f;
            cur->attributes.curHealth = cur->attributes.maxHealth;

            cur->attributes.maxMana = 100.0f;
            cur->attributes.curMana = cur->attributes.maxMana;

            cur->attributes.baseDamage = 5.0f;
            cur->attributes.attackChance = 80;
            cur->attributes.criticalChance = 5;
            cur->attributes.criticalModifier = 1.5f;
            break;

        case DS_SkeletonElite:
            cur->base.name = "Royal  Skeleton";
            cur->isBoss = true;
            cur->bossPreventClimbingLaddersWhileFighting = true;
            cur->bossPreventOpeningDoorsWhileFighting = true;

            cur->BehaviourUpdate = G_AI_BehaviourMeeleEnemy;
            
            cur->speed = 4.0f;
            cur->attributes.maxHealth = 700.0f;
            cur->attributes.curHealth = cur->attributes.maxHealth;

            cur->attributes.maxMana = 700.0f;
            cur->attributes.curMana = cur->attributes.maxMana;

            cur->attributes.baseDamage = 7.0f;
            cur->attributes.attackChance = 90;
            cur->attributes.criticalChance = 10;
            cur->attributes.criticalModifier = 1.5f;
            break;

        case DS_SkeletonBurnt:
            cur->base.name = "Burnt Skeleton";
            cur->isBoss = false;

            cur->BehaviourUpdate = G_AI_BehaviourCasterEnemy;
            cur->spellInUse = SPELL_FIREBALL1;

            cur->speed = 3.0f;
            cur->attributes.maxHealth = 50.0f;
            cur->attributes.curHealth = cur->attributes.maxHealth;

            cur->attributes.maxMana = 100.0f;
            cur->attributes.curMana = cur->attributes.maxMana;

            cur->attributes.baseDamage = 5.0f;
            cur->attributes.attackChance = 80;
            cur->attributes.criticalChance = 5;
            cur->attributes.criticalModifier = 1.5f;
            break;

        default:
            printf("AI with ID %d was not initalized. Setting it with base stats.\n");
            cur->isBoss = false;
            cur->BehaviourUpdate = G_AI_BehaviourMeeleEnemy;
            cur->attributes.maxHealth = 100.0f;
            cur->attributes.curHealth = cur->attributes.maxHealth;

            cur->attributes.maxMana = 100.0f;
            cur->attributes.curMana = cur->attributes.maxMana;

            cur->attributes.baseDamage = 5.0f;
            cur->attributes.attackChance = 80;
            cur->attributes.criticalChance = 5;
            cur->attributes.criticalModifier = 1.5f;
        break;
    }

    cur->aggroedPlayer = false;
    

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

        cur->BehaviourUpdate(cur);
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

    // Check if this was the boss the player was fighting, if he was, release the UI
    if(player.isFightingBoss && player.bossFighting == cur)
    {
        player.isFightingBoss = false;
        player.bossFighting = NULL;
    }
}


void G_AIAttackPlayer(dynamicSprite_t* cur)
{
    // Attack chance
    int attack      =  (rand() % (100)) + 1;
    int critical    =  (rand() % (100)) + 1;

    if(attack <= cur->attributes.attackChance)
    {
        float dmg = cur->attributes.baseDamage;

        if(critical <= cur->attributes.criticalChance)
            dmg *= cur->attributes.criticalModifier;

        G_PlayerTakeDamage(dmg);
    }
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
            cur->animSpeed = ANIMATION_SPEED_DIVIDER;
            break;

        case ANIM_DIE:
            cur->state = DS_STATE_DEAD;
            cur->animSpeed = ANIMATION_SPEED_DIVIDER;
            break;

        case ANIM_ATTACK1:
            cur->state = DS_STATE_ATTACKING;
            cur->animSpeed = ANIMATION_ATTACK_SPEED_DIVIDER;
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
            cur->animSpeed = ANIMATION_SPEED_DIVIDER;
            break;

        case ANIM_DIE:
            cur->state = DS_STATE_DEAD;
            cur->animSpeed = ANIMATION_SPEED_DIVIDER;
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

bool G_AICanAttack(dynamicSprite_t* cur)
{
    return (cur->state != DS_STATE_ATTACKING && cur->state != DS_STATE_DEAD);
}


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