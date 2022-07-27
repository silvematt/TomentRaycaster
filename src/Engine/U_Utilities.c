#include "U_Utilities.h"

void U_SetBit(byte* b, byte n)
{
    *b |= 1 << n;
}

byte U_GetBit(byte* b, byte n)
{
    return (*b >> n) & 1;
}