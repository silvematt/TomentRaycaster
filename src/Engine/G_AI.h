#ifndef AI_H_INCLUDED
#define AI_H_INCLUDED

#include "D_AssetsManager.h"
#include "U_DataTypes.h"

#define AI_STOP_DISTANCE 90.0f
#define AI_MELEE_ATTACK_DISTANCE 100.0f

#define AI_SPELL_ATTACK_DISTANCE 500.0f


// Dynamic AI list
extern dynamicSprite_t* allDynamicSprites[OBJECTARRAY_DEFAULT_SIZE_HIGH];
extern unsigned int allDynamicSpritesLength;

void G_AIInitialize(dynamicSprite_t* cur, int level, int spriteID, int x, int y);
void G_AIUpdate(void);
void G_AIDie(dynamicSprite_t* cur);
void G_AIPlayAnimationOnce(dynamicSprite_t* cur, objectanimationsID_e animID);
void G_AIPlayAnimationLoop(dynamicSprite_t* cur, objectanimationsID_e animID);
void G_AITakeDamage(dynamicSprite_t* cur, float amount);
bool G_AICanAttack(dynamicSprite_t* cur);
void G_AIAttackPlayer(dynamicSprite_t* cur);

#endif
