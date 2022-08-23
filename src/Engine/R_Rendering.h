#ifndef RENDERING_H_INCLUDED
#define RENDERING_H_INCLUDED

#include "../include/SDL2/SDL.h"

#include "M_Map.h"
#include "U_DataTypes.h"

// =========================================
// Raycasting
// =========================================
#define PLAYER_FOV 60                   // FOV of the player for raycasting
#define PLAYER_FOV_F 60.0f              // FOV of the player for raycasting (as float)
#define PROJECTION_PLANE_WIDTH 640     // Projection Plane
#define PROJECTION_PLANE_HEIGHT 480
#define PROJECTION_PLANE_CENTER 240
//#define DISTANCE_TO_PROJECTION ((PROJECTION_PLANE_WIDTH / 2) / tan(PLAYER_FOV /2))
#define DISTANCE_TO_PROJECTION 554      // Distance to projection
#define RENDERING_PLAYER_HEIGHT 32
//#define GAME_VIEW_OFFSETX 350           // Offset of the GameView
//#define GAME_VIEW_OFFSETY 125           // Offset of the GameView

// =========================================
// Minimap
// =========================================
#define MINIMAP_DIVIDER 32               // Divider for making the minimap smaller
#define MINIMAP_PLAYER_WIDTH 4          // Dividers for making the player in the minimap smaller
#define MINIMAP_PLAYER_HEIGHT 4

// =========================================
// Sprites
// =========================================
#define MAXVISABLE 50
#define MAX_SPRITE_HEIGHT 1000
#define ANIMATION_SPEED_DIVIDER 200

// Visible Sprite Determination
extern bool visibleTiles[MAP_HEIGHT][MAP_WIDTH];
extern sprite_t visibleSprites[MAXVISABLE];
extern int visibleSpritesLength;

// =========================================
// Thin wall Transparency
// =========================================

// Max amounts of times the ray can ignore and save a thin wall (see through a see through wall)
#define MAX_THIN_WALL_TRANSPARENCY_RECURSION 4

// Found thin walls to draw
extern walldata_t currentThinWalls[PROJECTION_PLANE_WIDTH * MAX_THIN_WALL_TRANSPARENCY_RECURSION];
extern unsigned visibleThinWallsLength;

// =========================================
// Debug
// =========================================
#define DEBUG_RAYCAST_MINIMAP 0
#define DEBUG_VISIBLE_TILES_MINIMAP 0
#define DEBUG_VISIBLE_SPRITES_MINIMAP 0

extern uint32_t r_blankColor;           // Color shown when nothing else is in the renderer
extern uint32_t r_transparencyColor;    // Color marked as "transparency", rendering of this color will be skipped for surfaces

extern unsigned int* screenBuffers[5];  // Buffers for the screen renderer
extern SDL_Rect dirtybox;               // Marks the dirty pixels, used for optimization

// Wall heights, saved for each x
extern float wallHeights[PROJECTION_PLANE_WIDTH];

// Drawables
#define MAX_DRAWABLES PROJECTION_PLANE_WIDTH * MAX_THIN_WALL_TRANSPARENCY_RECURSION + MAXVISABLE
extern drawabledata_t allDrawables[MAX_DRAWABLES];
extern int allDrawablesLength;

//-------------------------------------
// Initializes the rendering 
//-------------------------------------
void R_InitRendering(void);

//-------------------------------------
// Fill buffers and put framebuffers on top of each other
//-------------------------------------
void R_ComposeFrame(void);

//-------------------------------------
// Reads the framebuffer transfers to win_surface
//-------------------------------------
void R_UpdateNoBlit(void);

//-------------------------------------
// Updates the screen to the win_surface
//-------------------------------------
void R_FinishUpdate(void);

//-------------------------------------
// Gets a pixel from a surface
//-------------------------------------
Uint32 R_GetPixelFromSurface(SDL_Surface *surface, int x, int y);

//-------------------------------------
// Given an SDL_Surface, extracts the pixels of it and puts them in the selected framebuffer
//-------------------------------------
void R_BlitIntoBuffer(int buffer, SDL_Surface* sur, SDL_Rect* pos);

//-------------------------------------
// Given a color, extracts draws it in the selected framebuffer
//-------------------------------------
void R_BlitColorIntoBuffer(int buffer, int color, SDL_Rect* pos);

//-------------------------------------
// Draw lines using Bresenham's 
//-------------------------------------
void R_DrawLine(int x0, int y0, int x1, int y1, int color);

//-------------------------------------
// Draw a single pixel
//-------------------------------------
void R_DrawPixel(int x, int y, int color);

//-------------------------------------
// Draw a single pixel with shading
//-------------------------------------
void R_DrawPixelShaded(int x, int y, int color, float intensity);

//-------------------------------------
// Draw a column of pixel
//-------------------------------------
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

//-------------------------------------
// Drawables routine, sort and render drawables
//-------------------------------------
void R_DrawDrawables(void);

//-------------------------------------
// Draw the passed thin wall
//-------------------------------------
void R_DrawThinWall(walldata_t* wall);

//-------------------------------------
// Floorcast and ceilingcast
// Params:
// - end = the end of the wall that states where to start to floorcast
// - rayAngle = the current rayangle
// - x = the x coordinate on the screen for this specific floor cast call
//-------------------------------------
void R_FloorCastingAndCeiling(float end, float rayAngle, int x, float wallHeight);


//-------------------------------------
// Adds a sprite to the visible sprite array and adds its corresponding drawable
//-------------------------------------
void R_AddToVisibleSprite(int gridX, int gridY);


//-------------------------------------
// Draws the visible sprites
//-------------------------------------
void R_DrawSprite(sprite_t* sprite);

//-------------------------------------
// Draws a column of pixels with texture mapping
//-------------------------------------
void R_DrawColumnTextured(int x, int y, int endY, SDL_Surface* texture, int xOffset, float wallheight);

//-------------------------------------
// Draws a column of pixels with texture mapping and shading
//-------------------------------------
void R_DrawColumnTexturedShaded(int x, int y, int endY, SDL_Surface* texture, int xOffset, float wallheight, float intensity);

#endif