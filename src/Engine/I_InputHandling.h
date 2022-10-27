#ifndef INPUT_HANDLING_H_INCLUDED
#define INPUT_HANDLING_H_INCLUDED

#include "../include/SDL2/SDL.h"

#include "A_Application.h"
#include "G_Player.h"
#include "G_MainMenu.h"

// -------------------------------
// Holds the player's input
// -------------------------------
typedef struct playerinput_s
{
    vector2_t input;
    vector2_t mouseInput;
    vector2_t dir;
    vector2_t strafe; // 1 = right -1 = left
} playerinput_t;

extern playerinput_t playerinput;

void I_HandleInputMenu(void);

// -------------------------------
// Handles SDL Events and input
// -------------------------------
void I_HandleInputGame(void);

#endif