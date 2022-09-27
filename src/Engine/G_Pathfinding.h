#ifndef PATHFINDING_H_INCLUDED
#define PATHFINDING_H_INCLUDED

#include <stdbool.h>

#include "M_Map.h"
#include "U_DataTypes.h"


extern pathnode_t frontier[MAP_HEIGHT*MAP_WIDTH+2];
extern unsigned int frontierLength;
extern bool visited[MAP_HEIGHT][MAP_WIDTH];

extern bool G_CheckDynamicSpriteMap(int level, int y, int x);
extern dynamicSprite_t* G_GetFromDynamicSpriteMap(int level, int y, int x);
extern void G_ClearFromDynamicSpriteMap(int level, int y, int x);

extern void G_AddToDeadDynamicSpriteMap(dynamicSprite_t* cur, int level, int y, int x);
extern bool G_CheckDeadDynamicSpriteMap(int level, int y, int x);
extern dynamicSprite_t* G_GetFromDeadDynamicSpriteMap(int level, int y, int x);
extern void G_ClearFromDeadDynamicSpriteMap(int level, int y, int x);

//----------------------------------
// Performs pathfinding and returns a path 
//----------------------------------
extern path_t G_PerformPathfinding(int level, vector2Int_t gridPos, vector2Int_t gridTargetPos, dynamicSprite_t* entity);

//----------------------------------
// Performs the pathfinding between the first dynamic sprite and the player and debugs the info in the minimap, called via F2 (Player.c)
//----------------------------------
extern path_t G_PerformPathfindingDebug(int level, vector2Int_t gridPos, vector2Int_t gridTargetPos);

#endif