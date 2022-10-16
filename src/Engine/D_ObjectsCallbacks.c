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
        printf("Adding Weapon: Axe \n");

        alertMessage_t* mess = (alertMessage_t*)malloc(sizeof(alertMessage_t));
        R_QueueAlertMessage(mess, ALERT_MESSAGE_DEF_X, ALERT_MESSAGE_DEF_Y, "You picked up an Axe", 2.0f, 1.0f);

        player.hasAxe = true;
        G_PlayerSetWeapon(PLAYER_FP_AXE);
    }
    else if(strcmp(data, "PICKUP_HEALTH") == 0)
    {
        printf("Adding player's health\n");

        alertMessage_t* mess = (alertMessage_t*)malloc(sizeof(alertMessage_t));
        R_QueueAlertMessage(mess, ALERT_MESSAGE_DEF_X-50, ALERT_MESSAGE_DEF_Y, "You drink an Health  Potion", 2.0f, 1.0f);

        G_PlayerGainHealth(20.0f);
    }
    else if(strcmp(data, "PICKUP_MANA") == 0)
    {
        printf("Adding player's mana\n");

        alertMessage_t* mess = (alertMessage_t*)malloc(sizeof(alertMessage_t));
        R_QueueAlertMessage(mess, ALERT_MESSAGE_DEF_X-50, ALERT_MESSAGE_DEF_Y, "You drink a  Mana  Potion", 2.0f, 1.0f);
        G_PlayerGainMana(20.0f);
    }
    else if(strcmp(data, "TOME_FIREBALL") == 0)
    {
        printf("Adding Spell: Fireball\n");

        alertMessage_t* mess = (alertMessage_t*)malloc(sizeof(alertMessage_t));
        R_QueueAlertMessage(mess, ALERT_MESSAGE_DEF_X-50, ALERT_MESSAGE_DEF_Y, "You learn:  Fireball!", 2.0f, 1.0f);
        player.hasFireball = true;
        G_PlayerSetSpell(SPELL_FIREBALL1);
    }
    else if(strcmp(data, "TOME_ICEDART") == 0)
    {
        printf("Adding Spell: Ice Dart\n");

        alertMessage_t* mess = (alertMessage_t*)malloc(sizeof(alertMessage_t));
        R_QueueAlertMessage(mess, ALERT_MESSAGE_DEF_X-50, ALERT_MESSAGE_DEF_Y, "You learn:  Ice  Dart!", 2.0f, 1.0f);
        player.hasIceDart = true;
        G_PlayerSetSpell(SPELL_ICEDART1);
    }
}

void D_CallbackLadder(char* data)
{
    printf("Climbing Ladder\n");
}