#include "G_MainMenu.h"
#include "G_Game.h"
#include "R_Rendering.h"
#include "T_TextRendering.h"
#include "D_AssetsManager.h"

// ----------------------------
// Define Menus
// ----------------------------
menuelement_t MainMenuElements[] =
{
    {"New  Game",   {220, 150, 400, 40}, CALLBACK_MAINMENU_NewGame},
    {"Load  Game",  {220, 200, 400, 40}, NULL},
    {"Options",     {220, 250, 400, 40}, NULL},
    {"About",       {220, 300, 400, 40}, NULL},
    {"Quit",        {220, 350, 400, 40}, CALLBACK_MAINMENU_Quit}
};

menu_t MainMenu = {MENU_START, MainMenuElements, 5, &MainMenuElements[0]};


menu_t* currentMenu = &MainMenu;

//-------------------------------------
// Renders what's gonna be behind the current menu
//-------------------------------------
void G_RenderCurrentMenuBackground(void)
{
    switch(currentMenu->ID)
    {
        case MENU_START:
        {
            SDL_Rect titleRect = {110, 10, tomentdatapack.uiAssets[M_ASSET_TITLE]->texture->w, tomentdatapack.uiAssets[M_ASSET_TITLE]->texture->h};
            R_BlitIntoScreenScaled(NULL, tomentdatapack.uiAssets[M_ASSET_TITLE]->texture, &titleRect);
            break;
        }
    }
}

//-------------------------------------
// Render menu routine
//-------------------------------------
void G_RenderCurrentMenu(void)
{
    // Display text
    for(int i = 0; i < 5; i++)
    {
        T_DisplayTextScaled(FONT_BLKCRY, MainMenu.elements[i].text, MainMenu.elements[i].box.x, MainMenu.elements[i].box.y, 1.25f);
    }

    // Display cursor
    SDL_Rect cursorRect = {currentMenu->selectedElement->box.x - CURSOR_X_OFFSET, currentMenu->selectedElement->box.y, tomentdatapack.uiAssets[M_ASSET_SELECT_CURSOR]->texture->w, tomentdatapack.uiAssets[M_ASSET_SELECT_CURSOR]->texture->h};
    R_BlitIntoScreen(NULL, tomentdatapack.uiAssets[M_ASSET_SELECT_CURSOR]->texture, &cursorRect);
}

void G_InMenuInputHandling(SDL_Event* e)
{
    // Offset mouse pos
    static int x = 0;
    static int y = 0;

    //Get the mouse offsets
    x = e->button.x;
    y = e->button.y;

    for(int i = 0; i < currentMenu->elementsLength; i++)
    {
        bool isOn = (( x > currentMenu->elements[i].box.x) && ( x < currentMenu->elements[i].box.x + currentMenu->elements[i].box.w ) && 
                     ( y > currentMenu->elements[i].box.y) && ( y < currentMenu->elements[i].box.y + currentMenu->elements[i].box.h ));

        if(e->type == SDL_MOUSEMOTION)
        {
            // Select hovering element
            if (isOn)
                currentMenu->selectedElement = &currentMenu->elements[i];
        }
        else if(e->type == SDL_MOUSEBUTTONUP && e->button.button == SDL_BUTTON_LEFT)
        {
            if (isOn && currentMenu->selectedElement->OnClick != NULL)
            {
                currentMenu->selectedElement->OnClick();
            }
        }
    }
}

//-------------------------------------
// BUTTONS CALLBACKS
//-------------------------------------
//-------------------------------------
// MENU_START_NEW_GAME
//-------------------------------------
static void CALLBACK_MAINMENU_NewGame(void)
{
    // Initialize the player
    player.hasBeenInitialized = false;

    G_InitGame();
    A_ChangeState(GSTATE_GAME);
}


//-------------------------------------
// MENU_QUIT
//-------------------------------------
static void CALLBACK_MAINMENU_Quit(void)
{
    application.quit = true;
}