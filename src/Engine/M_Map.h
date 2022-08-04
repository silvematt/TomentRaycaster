#ifndef MAP_H_INCLUDED
#define MAP_H_INCLUDED

#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "A_Application.h"
#include "D_AssetsManager.h"
#include "U_Utilities.h"

// Max Map Width/Height
#define MAP_WIDTH 24
#define MAP_HEIGHT 24
#define TILE_SIZE 64

#define DEVMAP 0

typedef enum objectType_e
{
    ObjT_Empty = 0,
    ObjT_Wall,
    ObjT_Door,
    ObjT_Sprite
} objectType_e;

typedef struct map_s
{
    char id[MAX_STRLEN];
    char name[MAX_STRLEN];
    int wallMap[MAP_HEIGHT][MAP_WIDTH];
    int floorMap[MAP_HEIGHT][MAP_WIDTH];
    int ceilingMap[MAP_HEIGHT][MAP_WIDTH];
    int spritesMap[MAP_HEIGHT][MAP_WIDTH];
    float wallLight;
    float floorLight;

    int objectTMap[MAP_HEIGHT][MAP_WIDTH];    // Contains the objectType_e currently placed in that cell, mainly used to differientate input and to know which kind of object is where
    int collisionMap[MAP_HEIGHT][MAP_WIDTH];
} map_t;

extern map_t currentMap;

void M_LoadMapAsCurrent(char* mapID);
void M_LoadObjectTMap(void);
void M_LoadCollisionMap(void);

#endif