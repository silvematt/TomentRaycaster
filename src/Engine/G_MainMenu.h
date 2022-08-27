#ifndef MAIN_MENU_H_INCLUDED
#define MAIN_MENU_H_INCLUDED

#include <stdbool.h>

#include "../include/SDL2/SDL.h"

// How off the text the cursor should be
#define CURSOR_X_OFFSET 35

typedef enum
{
    MENU_START = 0
} menusIDs_e;


typedef struct menuelement_s
{
    char* text;
    SDL_Rect box;
    void (*OnClick)(void);
} menuelement_t;

typedef struct menu_s
{
    int ID;
    menuelement_t* elements;
    int elementsLength;
    menuelement_t* selectedElement;
} menu_t;


extern menu_t* currentMenu;

// Menus are initialized at compile-time
extern menuelement_t MainMenuElements[];
extern menu_t MainMenu;


//-------------------------------------
// Renders what's gonna be behind the current menu
//-------------------------------------
void G_RenderCurrentMenuBackground(void);


//-------------------------------------
// Render menu routine, renders the elements of any menu
//-------------------------------------
void G_RenderCurrentMenu(void);


//-------------------------------------
// Manages Menu Input routine
//-------------------------------------
void G_InMenuInputHandling(SDL_Event* e);


//-------------------------------------
// BUTTONS CALLBACKS
//-------------------------------------
//-------------------------------------
// MENU_START_NEW_GAME
//-------------------------------------
static void CALLBACK_MAINMENU_NewGame(void);

//-------------------------------------
// MENU_QUIT
//-------------------------------------
static void CALLBACK_MAINMENU_Quit(void);

#endif
