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
    vector2_t pos;
    vector2Int_t gridPos;
    int spriteID;
    byte flags;
} sprite_t;

#endif