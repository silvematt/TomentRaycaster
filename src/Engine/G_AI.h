#ifndef AI_H_INCLUDED
#define AI_H_INCLUDED

#include "D_AssetsManager.h"
#include "U_DataTypes.h"

// Dynamic AI list
extern dynamicSprite_t* allDynamicSprites[OBJECTARRAY_DEFAULT_SIZE];
extern unsigned int allDynamicSpritesLength;

void G_AIInitialize(dynamicSprite_t* cur, int level, int spriteID, int x, int y);
void G_AIUpdate(void);
void G_AIDie(dynamicSprite_t* cur);
void G_PlayAnimationOnce(dynamicSprite_t* cur, objectanimationsID_e animID);

#endif
