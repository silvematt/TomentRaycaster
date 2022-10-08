#include <stdio.h>
#include <string.h>
#include "G_Game.h"
#include "D_ObjectsCallbacks.h"

void D_CallbackChangeMap(char* data)
{
    G_ChangeMap(data);
}

void D_CallbackPickup(char* data)
{
    if(strcmp(data, "WEAPON_AXE") == 0)
    {
        printf("Adding axe to player's inventory\n");
    }
    else if(strcmp(data, "PICKUP_HEALTH") == 0)
    {
        printf("Adding player's health\n");
        G_PlayerGainHealth(20.0f);
    }
    else if(strcmp(data, "PICKUP_MANA") == 0)
    {
        printf("Adding player's mana\n");
        G_PlayerGainMana(20.0f);
    }
}