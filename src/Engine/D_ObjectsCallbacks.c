#include<stdio.h>
#include "G_Game.h"
#include "D_ObjectsCallbacks.h"

void CastleDoorsLvl2Callback(char* data)
{
    G_ChangeMap(data);
}
