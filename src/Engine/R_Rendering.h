#ifndef RENDERING_H_INCLUDED
#define RENDERING_H_INCLUDED

#include "../include/SDL2/SDL.h"


#define PLAYER_FOV 60                   // FOV of the player for raycasting
#define PROJECTION_PLANE_WIDTH 640      // Projection Plane
#define PROJECTION_PLANE_HEIGHT 480
#define PROJECTION_PLANE_CENTER 240
//#define DISTANCE_TO_PROJECTION ((PROJECTION_PLANE_WIDTH / 2) / tan(PLAYER_FOV /2))
#define DISTANCE_TO_PROJECTION 554      // Distance to projection

//#define GAME_VIEW_OFFSETX 350           // Offset of the GameView
//#define GAME_VIEW_OFFSETY 125           // Offset of the GameView

#define MINIMAP_DIVIDER 16               // Divider for making the minimap smaller
#define MINIMAP_PLAYER_WIDTH 4          // Dividers for making the player in the minimap smaller
#define MINIMAP_PLAYER_HEIGHT 4

// =========================================
// Debug
// =========================================
#define DEBUG_RAYCAST_MINIMAP 0

extern uint32_t r_blankColor;           // Color shown when nothing else is in the renderer
extern uint32_t r_transparencyColor;    // Color marked as "transparency", rendering of this color will be skipped for surfaces

extern unsigned int* screenBuffers[5];  // Buffers for the screen renderer
extern SDL_Rect dirtybox;               // Marks the dirty pixels, used for optimization

//-------------------------------------
// Initializes the rendering 
//-------------------------------------
void R_InitRendering(void);

//-------------------------------------
// Fill buffers and put framebuffers on top of each other
//-------------------------------------
void R_ComposeFrame(void);

//-------------------------------------
// Reads the framebuffer with the dirtybox and transfer to win_surface
//-------------------------------------
void R_UpdateNoBlit(void);

//-------------------------------------
// Updates the screen to the win_surface
//-------------------------------------
void R_FinishUpdate(void);

//-------------------------------------
// Given an SDL_Surface, extracts the pixels of it and puts them in the selected framebuffer
//-------------------------------------
void R_BlitIntoBuffer(int buffer, SDL_Surface* sur, SDL_Rect* pos);

//-------------------------------------
// Given a color, extracts draws it in the selected framebuffer
//-------------------------------------
void R_BlitColorIntoBuffer(int buffer, int color, SDL_Rect* pos);

void R_DrawLine(int x0, int y0, int x1, int y1, int color);

void R_DrawPixel(int x, int y, int color);

void R_DrawColumn(int x, int y, int endY, int color);

//-------------------------------------
// Sets the screen to r_blankColor
//-------------------------------------
void R_ClearRendering(void);

//-------------------------------------
// Draws the minimap
//-------------------------------------
void R_DrawMinimap(void);

//-------------------------------------
// Draws the background of the game view
//-------------------------------------
void R_DrawBackground(void);

//-------------------------------------
// Performs the Raycast and draws the walls
//-------------------------------------
void R_Raycast(void);

void R_DrawColumnTextured(int x, int y, int endY, SDL_Surface* texture, int xOffset, float wallheight);

#endif