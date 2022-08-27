#include "I_InputHandling.h"

playerinput_t playerinput;

// -------------------------------
// Handles SDL Events and input
// -------------------------------
void I_HandleInput(void)
{
    const Uint8* keyboard_state = SDL_GetKeyboardState(NULL);
    const Uint32 mouse_state = SDL_GetMouseState(NULL, NULL);

    SDL_Event e;
    
    while(SDL_PollEvent(&e))
    {
        switch (e.type)
        {
            case SDL_QUIT:
                application.quit = true;
            break;
        }

        if(application.gamestate == GSTATE_GAME)
            G_InGameInputHandlingEvent(&e);
        else
            G_InMenuInputHandling(&e);
    }

    // Send Input event to subsystems
    if(application.gamestate == GSTATE_GAME)
        G_InGameInputHandling(keyboard_state, &e);
}