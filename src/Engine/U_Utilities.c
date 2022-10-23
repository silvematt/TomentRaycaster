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



// -------------------------------
// Perform quicksorts before drawing drawables to draw from farther to nearest
// -------------------------------
void U_QuicksortDrawables(drawabledata_t a[], int left, int right)
{
    int i = left;
    int j = right;
    
    drawabledata_t temp = a[i]; // temp and pivot

    if( left < right)
    {
        while(i < j)
        {
            while(a[j].dist <= temp.dist && i < j)
                j--;

            a[i] = a[j];

            while(a[i].dist >= temp.dist && i < j)
                i++;
            
            a[j] = a[i];
        }
        a[i] = temp;

        U_QuicksortDrawables(a, left, i - 1);
        U_QuicksortDrawables(a, j + 1, right);
    }
}

// -------------------------------
// Perform quicksorts before drawing drawables to draw from farther to nearest
// -------------------------------
void U_QuicksortWallData(walldata_t a[], int left, int right)
{
    int i = left;
    int j = right;
    
    walldata_t temp = a[i]; // temp and pivot

    if( left < right)
    {
        while(i < j)
        {
            while(a[j].distance <= temp.distance && i < j)
                j--;

            a[i] = a[j];

            while(a[i].distance >= temp.distance && i < j)
                i++;
            
            a[j] = a[i];
        }
        a[i] = temp;

        U_QuicksortWallData(a, left, i - 1);
        U_QuicksortWallData(a, j + 1, right);
    }
}