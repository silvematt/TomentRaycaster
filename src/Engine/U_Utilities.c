#include <math.h>
#include <stdio.h>

#include "U_Utilities.h"

void U_SetBit(byte* b, byte n)
{
    *b |= 1 << n;
}

byte U_GetBit(byte* b, byte n)
{
    return (*b >> n) & 1;
}

float U_GetDistanceSquared(float x1, float y1, float x2, float y2)
{
    // Translate to player's coordinates
    float dx = x1 - x2;
    float dy = y1 - y2;

    return sqrt(dx*dx + dy*dy);
}

void I_SwapSprites(sprite_t* x, sprite_t* y)
{
    sprite_t temp;
    temp = *x;
    *x = *y;
    *y = temp;
}

int I_QuicksortSpritesPartition(sprite_t visibleSprites[], int l, int h)
{
    float pivot = visibleSprites[l].dist; // pivot
    int i = l;
    int j = h;

    while (i < j) 
    {
        while (visibleSprites[i].dist > pivot)
            i++;

        while (visibleSprites[j].dist < pivot)
            j--;

        if (i < j) 
            I_SwapSprites(&visibleSprites[i], &visibleSprites[j]);
    }

    return j;
}

void U_QuicksortSprites(sprite_t visibleSprites[], int first, int last)
{
    if(first < last)
    {
        int pivot = I_QuicksortSpritesPartition(visibleSprites, first, last);
        U_QuicksortSprites(visibleSprites, first, pivot);
        U_QuicksortSprites(visibleSprites, pivot+1, last);
    }
}
