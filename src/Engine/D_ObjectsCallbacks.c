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
    wallObject_t* wallObj = R_GetWallObjectFromMap(player.level, player.inFrontGridPosition.y, player.inFrontGridPosition.x);

    // Check if he player is facing a ladder

    // If player is looking at the up face
    if(player.angle < (5*M_PI)/6 && player.angle > M_PI/6)
    {
        // Check if the texture at that point is the ladder
        if(U_GetBit(&tomentdatapack.textures[wallObj->texturesArray[TEXTURE_ARRAY_UP]]->flags, 0))
        {
            // Climb!!
            player.climbingPosX = player.position.x;
            player.climbingPosY = player.position.y + TILE_SIZE;
            player.climbingPosZ = player.z + TILE_SIZE;
            player.climbingUp = true;
            player.hasToClimb = true;
        }
    }
    // If player is looking at the bottom face 
    else if(player.angle > (7*M_PI)/6 && player.angle < (11*M_PI)/6)
    {
        // Check if the texture at that point is the ladder
        if(U_GetBit(&tomentdatapack.textures[wallObj->texturesArray[TEXTURE_ARRAY_DOWN]]->flags, 0))
        {
            // Climb!!
            player.climbingPosX = player.position.x;
            player.climbingPosY = player.position.y - TILE_SIZE;
            player.climbingPosZ = player.z + TILE_SIZE;
            player.climbingUp = true;
            player.hasToClimb = true;
        }
    }
    // If the player is looking at the left face
    else if(player.angle < M_PI/3 || player.angle > (5*M_PI) / 3)
    {
        // Check if the texture at that point is the ladder
        if(U_GetBit(&tomentdatapack.textures[wallObj->texturesArray[TEXTURE_ARRAY_LEFT]]->flags, 0))
        {
            // Climb!!
            player.climbingPosX = player.position.x + TILE_SIZE;
            player.climbingPosY = player.position.y;
            player.climbingPosZ = player.z + TILE_SIZE;
            player.climbingUp = true;
            player.hasToClimb = true;
        }
    }
    // If the player is looking at the right face
    else if(player.angle > (2*M_PI)/3 && player.angle < (4*M_PI)/3)
    {
        // Check if the texture at that point is the ladder
        if(U_GetBit(&tomentdatapack.textures[wallObj->texturesArray[TEXTURE_ARRAY_LEFT]]->flags, 0))
        {
            // Climb!!
            player.climbingPosX = player.position.x - TILE_SIZE;
            player.climbingPosY = player.position.y;
            player.climbingPosZ = player.z + TILE_SIZE;
            player.climbingUp = true;
            player.hasToClimb = true;
        }
    }
}

void D_CallbackLadderDown(char* data)
{
    printf("Climbing Ladde Downr\n");
    wallObject_t* wallObj = R_GetWallObjectFromMap(player.level, player.inFrontGridPosition.y, player.inFrontGridPosition.x);

    // Check if he player is facing a ladder

    // If player is looking at the up face
    if(player.angle < (5*M_PI)/6 && player.angle > M_PI/6)
    {
        // Check if the texture at that point is the ladder
        // Climb!!
        player.climbingPosX = player.position.x;
        player.climbingPosY = player.position.y + TILE_SIZE;
        player.climbingPosZ = player.z - TILE_SIZE;
        player.climbingUp = false;
        player.hasToClimb = true;
    }
    // If player is looking at the bottom face 
    else if(player.angle > (7*M_PI)/6 && player.angle < (11*M_PI)/6)
    {
        // Climb!!
        player.climbingPosX = player.position.x;
        player.climbingPosY = player.position.y - TILE_SIZE;
        player.climbingPosZ = player.z - TILE_SIZE;
        player.climbingUp = false;
        player.hasToClimb = true;
    }
    // If the player is looking at the left face
    else if(player.angle < M_PI/3 || player.angle > (5*M_PI) / 3)
    {
        // Check if the texture at that point is the ladder
        // Climb!!
        player.climbingPosX = player.position.x + TILE_SIZE;
        player.climbingPosY = player.position.y;
        player.climbingPosZ = player.z - TILE_SIZE;
        player.climbingUp = false;
        player.hasToClimb = true;
    }
    // If the player is looking at the right face
    else if(player.angle > (2*M_PI)/3 && player.angle < (4*M_PI)/3)
    {
        // Climb!!
        player.climbingPosX = player.position.x - TILE_SIZE;
        player.climbingPosY = player.position.y;
        player.climbingPosZ = player.z - TILE_SIZE;
        player.climbingUp = false;
        player.hasToClimb = true;
    }
}