#include "G_MainMenu.h"
#include "G_Game.h"
#include "R_Rendering.h"
#include "T_TextRendering.h"
#include "D_AssetsManager.h"

static void CALLBACK_Continue(void)
{
    if(player.hasBeenInitialized)
            A_ChangeState(GSTATE_GAME);
}

// ----------------------------
// Define Menus
// ----------------------------
menuelement_t MainMenuElements[] =
{
    {"Continue",    {220, 150, 400, 40}, CALLBACK_Continue},
    {"New  Game",   {220, 200, 400, 40}, CALLBACK_MAINMENU_NewGame},
    {"Load  Game",  {220, 250, 400, 40}, NULL},
    {"Options",     {220, 300, 400, 40}, CALLBACK_MAINMENU_Options},
    {"About",       {220, 350, 400, 40}, NULL},
    {"Quit",        {220, 400, 400, 40}, CALLBACK_MAINMENU_Quit}
};
menu_t MainMenu = {MENU_START, MainMenuElements, 6, &MainMenuElements[1]};

menuelement_t DeathMenuElements[] =
{
    {"Restart  Game",    {220, 200, 400, 40}, CALLBACK_MAINMENU_NewGame},
    {"Return",           {220, 250, 200, 40}, CALLBACK_ReturnToMainMenu},
};
menu_t DeathMenu = {MENU_DEATH, DeathMenuElements, 2, &DeathMenuElements[0]};

menuelement_t OptionsMenuElements[] =
{
    {"Graphics:",    {220, 200, 400, 40}, CALLBACK_OPTIONSMENU_ChangeGraphics},
    {"Return",       {220, 250, 200, 40}, CALLBACK_ReturnToMainMenu},
};
menu_t OptionsMenu = {MENU_OPTIONS, OptionsMenuElements, 2, &OptionsMenuElements[0]};

menu_t* currentMenu;


void G_InitMainMenu()
{
    G_SetMenu(&MainMenu);
}

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

        case MENU_OPTIONS:
        {
            T_DisplayTextScaled(FONT_BLKCRY, "Settings", 180, 80, 2.0f);

            // Display graphics current setting:
            switch(r_CurrentGraphicsSetting)
            {
                case GRAPHICS_LOW:
                    T_DisplayTextScaled(FONT_BLKCRY, "Low", 350, 205, 1.0f);
                    break;
                
                case GRAPHICS_MEDIUM:
                    T_DisplayTextScaled(FONT_BLKCRY, "Medium", 350, 205, 1.0f);
                    break;

                case GRAPHICS_HIGH:
                    T_DisplayTextScaled(FONT_BLKCRY, "High", 350, 205, 1.0f);
                    break;
            }
            break;
        }

        case MENU_DEATH:
        {
            T_DisplayTextScaled(FONT_BLKCRY, "You  died!", 180, 80, 2.0f);
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
    for(int i = 0; i < currentMenu->elementsLength; i++)
    {
        T_DisplayTextScaled(FONT_BLKCRY, currentMenu->elements[i].text, currentMenu->elements[i].box.x, currentMenu->elements[i].box.y, 1.25f);
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
                return;
            }
        }
    }
}

void G_SetMenu(menu_t* newMenu)
{
    currentMenu = newMenu;
    currentMenu->elementsLength = newMenu->elementsLength;
}

//-------------------------------------
// BUTTONS CALLBACKS
//-------------------------------------
static void CALLBACK_MAINMENU_NewGame(void)
{
    // Initialize the player
    player.hasBeenInitialized = false;

    G_InitGame();
    A_ChangeState(GSTATE_GAME);
}

static void CALLBACK_MAINMENU_Options(void)
{
    G_SetMenu(&OptionsMenu);
    A_ChangeState(GSTATE_MENU);
}


static void CALLBACK_MAINMENU_Quit(void)
{
    application.quit = true;
}


static void CALLBACK_ReturnToMainMenu(void)
{
    G_SetMenu(&MainMenu);
    A_ChangeState(GSTATE_MENU);
}

static void CALLBACK_OPTIONSMENU_ChangeGraphics(void)
{
    if(r_CurrentGraphicsSetting+1 < 3)
        r_CurrentGraphicsSetting++;
    else
        r_CurrentGraphicsSetting = 0;

    R_SetRenderingGraphics(r_CurrentGraphicsSetting);
    R_ClearRendering();
}
