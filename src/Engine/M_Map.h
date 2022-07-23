#ifndef MAP_H_INCLUDED
#define MAP_H_INCLUDED

#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "A_Application.h"

// Max Map Width/Height
#define MAP_WIDTH 24
#define MAP_HEIGHT 24
#define TILE_SIZE 64

#define DEVMAP 0

typedef struct map_s
{
    char id[MAX_STRLEN];
    char name[MAX_STRLEN];
    int wallMap[MAP_HEIGHT][MAP_WIDTH];
    int floorMap[MAP_HEIGHT][MAP_WIDTH];
    int ceilingMap[MAP_HEIGHT][MAP_WIDTH];
} map_t;

extern map_t currentMap;

void M_LoadMapAsCurrent(char* mapID);

#endif