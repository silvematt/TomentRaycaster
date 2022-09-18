#include<stdio.h>
#include "G_Game.h"
#include "D_ObjectsCallbacks.h"

void D_CallbackChangeMap(char* data)
{
    G_ChangeMap(data);
}
