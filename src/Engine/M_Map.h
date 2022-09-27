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
#define UNIT_SIZE 64

#define MAX_N_LEVELS 3

// Types of object
typedef enum objectType_e
{
    ObjT_Empty = 0,
    ObjT_Wall,
    ObjT_Door,
    ObjT_Sprite,
    ObjT_Trigger        // Triggers the callback
} objectType_e;

// Map info
typedef struct map_s
{
    // Map static
    char id[MAX_STRLEN];
    char name[MAX_STRLEN];

    int playerStartingLevel;
    int playerStartingGridX;
    int playerStartingGridY;
    float playerStartingRot;
    
    // Walls on levels
    int level0[MAP_HEIGHT][MAP_WIDTH];
    int level1[MAP_HEIGHT][MAP_WIDTH];
    int level2[MAP_HEIGHT][MAP_WIDTH];

    int floorMap[MAP_HEIGHT][MAP_WIDTH];
    int ceilingMap[MAP_HEIGHT][MAP_WIDTH];

    int spritesMapLevel0[MAP_HEIGHT][MAP_WIDTH];
    int spritesMapLevel1[MAP_HEIGHT][MAP_WIDTH];
    int spritesMapLevel2[MAP_HEIGHT][MAP_WIDTH];

    float wallLight;
    float floorLight;
    int skyID;

    int maxCeilingHeight;

    // Map Dynamics, values are cached in multiple matrixes to avoid having to calculate them each time
    // These values can change during runtime

    int objectTMapLevel0[MAP_HEIGHT][MAP_WIDTH];      // Contains the objectType_e currently placed in that cell, mainly used to differientate input and to know which kind of object is where
    int objectTMapLevel1[MAP_HEIGHT][MAP_WIDTH];      // Contains the objectType_e currently placed in that cell, mainly used to differientate input and to know which kind of object is where
    int objectTMapLevel2[MAP_HEIGHT][MAP_WIDTH];      // Contains the objectType_e currently placed in that cell, mainly used to differientate input and to know which kind of object is where

    int collisionMapLevel0[MAP_HEIGHT][MAP_WIDTH];    // Collision data
    int collisionMapLevel1[MAP_HEIGHT][MAP_WIDTH];    // Collision data
    int collisionMapLevel2[MAP_HEIGHT][MAP_WIDTH];    // Collision data

    // Dynamic sprites, like AI
    dynamicSprite_t* dynamicSpritesLevel0[MAP_HEIGHT][MAP_WIDTH];
    dynamicSprite_t* dynamicSpritesLevel1[MAP_HEIGHT][MAP_WIDTH];
    dynamicSprite_t* dynamicSpritesLevel2[MAP_HEIGHT][MAP_WIDTH];

    // Death sprites
    dynamicSprite_t* deadDynamicSpritesLevel0[MAP_HEIGHT][MAP_WIDTH];
    dynamicSprite_t* deadDynamicSpritesLevel1[MAP_HEIGHT][MAP_WIDTH];
    dynamicSprite_t* deadDynamicSpritesLevel2[MAP_HEIGHT][MAP_WIDTH];

} map_t;

// The currently loaded map
extern map_t currentMap;

// -------------------------------
// Loads the map from the file named mapID
// -------------------------------
void M_LoadMapAsCurrent(char* mapID);

// -------------------------------
// Loads the object map
// -------------------------------
void M_LoadObjectTMap(void);

// -------------------------------
// Loads the collision map
// -------------------------------
void M_LoadCollisionMaps(void);

#endif