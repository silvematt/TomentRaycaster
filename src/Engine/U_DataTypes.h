#ifndef DATA_TYPES_H_INCLUDED
#define DATA_TYPES_H_INCLUDED

#define RADIAN 0.0174533        // 1 Radian

#include <stdint.h>

typedef	uint8_t byte;

typedef struct vector2_s
{
    float x, y;
} vector2_t;

typedef struct vector2Int_s
{
    int x,y;
} vector2Int_t;

typedef struct sprite_s
{
    vector2_t pos;          // position in world
    vector2Int_t gridPos;   // position in grid
    vector2_t pSpacePos;    // position in player space        

    int spriteID;
    byte flags;

    float dist;     // distance from player
    float height;   // how big the sprite will be drawn
} sprite_t;

typedef enum doorstate_e
{
    DState_Closed = 0,
    DState_Opening,
    DState_Open,
    DState_Closing
} doorstate_e;

#endif