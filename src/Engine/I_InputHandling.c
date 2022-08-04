#include "I_InputHandling.h"

playerinput_t playerinput;

// -------------------------------
// Handles SDL Events and input
// -------------------------------
void I_HandleInput(void)
{
    const Uint8* keyboard_state = SDL_GetKeyboardState(NULL);

    SDL_Event e;
    
    while(SDL_PollEvent(&e))
    {
        switch (e.type)
        {
            case SDL_QUIT:
                application.quit = true;
            break;

        }

        G_PlayerHandleInputEvent(&e);
    }

    // Send Input event to subsystems
    G_PlayerHandleInput(keyboard_state, &e);
}