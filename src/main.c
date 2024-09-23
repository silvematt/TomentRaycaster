// Standard Libraries
#include <stdio.h>

// External Libraries
#include "SDL.h"
#undef main

// Engine
#include "Engine/A_Application.h"
#include "Engine/I_InputHandling.h"
#include "Engine/M_Map.h"
#include "Engine/G_Player.h"
#include "Engine/G_Game.h"
#include "Engine/D_AssetsManager.h"

// ------------------------------------------------
//  Source Code 
// ------------------------------------------------
//  A_ [ Application/Implementation Specific]
//  G_ [Game]
//  I_ [Input/Implementation Specific]
//  R_ [Rendering]
//  P_ [Physics]
//  M_ [Map]
//  D_ [Game data/Asset management]
//  U_ [Utilities]
// ------------------------------------------------

int main(int argc, char* argv[])
{
    // Absolute initializations
    A_InitApplication();
    D_InitAssetManager();
    G_InitMainMenu();
    
    // Loop
    while(!application.quit)
    {
        G_GameLoop();
    }

    // Exit
    A_QuitApplication();
    return 0;
}