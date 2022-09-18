#ifndef PATHFINDING_H_INCLUDED
#define PATHFINDING_H_INCLUDED

#include <stdbool.h>

#include "M_Map.h"
#include "U_DataTypes.h"


extern pathnode_t frontier[MAP_HEIGHT*MAP_WIDTH+2];
extern unsigned int frontierLength;
extern bool visited[MAP_HEIGHT][MAP_WIDTH];

//----------------------------------
// Performs pathfinding and returns a path 
//----------------------------------
extern path_t G_PerformPathfinding(int level, vector2Int_t gridPos, vector2Int_t gridTargetPos);

//----------------------------------
// Performs the pathfinding between the first dynamic sprite and the player and debugs the info in the minimap, called via F2 (Player.c)
//----------------------------------
extern path_t G_PerformPathfindingDebug(int level, vector2Int_t gridPos, vector2Int_t gridTargetPos);

#endif