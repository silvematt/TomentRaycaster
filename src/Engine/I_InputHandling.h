#ifndef INPUT_HANDLING_H_INCLUDED
#define INPUT_HANDLING_H_INCLUDED

#include "../include/SDL2/SDL.h"

#include "A_Application.h"
#include "G_Player.h"

typedef struct playerinput_s
{
    vector2_t input;
    vector2_t dir;
    vector2_t strafe; // 1 = right -1 = left
} playerinput_t;

extern playerinput_t playerinput;

// -------------------------------
// Handles SDL Events and input
// -------------------------------
void I_HandleInput(void);

#endif