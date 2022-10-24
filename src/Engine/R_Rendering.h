#ifndef RENDERING_H_INCLUDED
#define RENDERING_H_INCLUDED

#include "../include/SDL2/SDL.h"

#include "M_Map.h"
#include "U_DataTypes.h"
#include <float.h>

// =========================================
// Raycasting
// =========================================
#define PLAYER_FOV 60                   // FOV of the player for raycasting
#define PLAYER_FOV_F 60.0f              // FOV of the player for raycasting (as float)


// Max values the projection plane can be
#define MAX_PROJECTION_PLANE_WIDTH 640
#define MAX_PROJECTION_PLANE_HEIGHT 480

// Runtime Graphichs
typedef enum GraphicsOptions_e
{
    GRAPHICS_LOW = 0,
    GRAPHICS_MEDIUM,
    GRAPHICS_HIGH
} GraphicsOptions_e;

extern GraphicsOptions_e r_CurrentGraphicsSetting;

// Projection Plane
extern int PROJECTION_PLANE_WIDTH;     
extern int PROJECTION_PLANE_HEIGHT;
extern int PROJECTION_PLANE_CENTER;

//#define DISTANCE_TO_PROJECTION ((PROJECTION_PLANE_WIDTH / 2) / tan(PLAYER_FOV /2))
extern int DISTANCE_TO_PROJECTION;

// Player's look up/down
extern int MAX_VERTICAL_HEAD_MOV;
extern int MIN_VERTICAL_HEAD_MOV;

// =========================================
// Minimap
// =========================================
#define MINIMAP_DIVIDER 16               // Divider for making the minimap smaller
#define MINIMAP_PLAYER_WIDTH 4          // Dividers for making the player in the minimap smaller
#define MINIMAP_PLAYER_HEIGHT 4

// =========================================
// Sprites
// =========================================
#define MAXVISABLE 50 // * MAX_FLOORS, the visibleSprites array and visibleTiles are reused each time
#define MAX_SPRITE_HEIGHT 2000
#define ANIMATION_SPEED_DIVIDER 200
#define ANIMATION_ATTACK_SPEED_DIVIDER 300

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
extern walldata_t currentThinWalls[MAX_PROJECTION_PLANE_WIDTH * MAX_THIN_WALL_TRANSPARENCY_RECURSION];
extern unsigned visibleThinWallsLength;

// Alert message
#define ALERT_MESSAGE_DEF_X 360
#define ALERT_MESSAGE_DEF_Y 10

extern alertMessage_t* alertMessagesHead;


// =========================================
// Debug
// =========================================
#define DEBUG_RAYCAST_MINIMAP 0
#define DEBUG_VISIBLE_TILES_MINIMAP 1
#define DEBUG_DYNAMIC_SPRITES 0
#define DEBUG_VISIBLE_SPRITES_MINIMAP 0

extern uint32_t r_blankColor;           // Color shown when nothing else is in the renderer
extern uint32_t r_transparencyColor;    // Color marked as "transparency", rendering of this color will be skipped for surfaces

// Wall heights, saved for each x for each level
extern float zBuffer[MAX_PROJECTION_PLANE_HEIGHT][MAX_PROJECTION_PLANE_WIDTH];

// Drawables
#define MAX_DRAWABLES MAX_PROJECTION_PLANE_WIDTH * MAX_THIN_WALL_TRANSPARENCY_RECURSION + MAXVISABLE
extern drawabledata_t allDrawables[MAX_DRAWABLES];
extern int allDrawablesLength;

extern bool debugRendering;
extern bool r_debugPathfinding;



//-------------------------------------
// Initializes the rendering 
//-------------------------------------
void R_InitRendering(void);

void R_SetRenderingGraphics(GraphicsOptions_e setting);

//-------------------------------------
// Fill buffers and put framebuffers on top of each other
//-------------------------------------
void R_ComposeFrame(void);


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
void R_BlitIntoScreen(SDL_Rect* size, SDL_Surface* sur, SDL_Rect* pos);

//-------------------------------------
// Given an SDL_Surface, extracts the pixels of it and puts them in the selected framebuffer
//-------------------------------------
void R_BlitIntoScreenScaled(SDL_Rect* size, SDL_Surface* sur, SDL_Rect* pos);

//-------------------------------------
// Given a color, extracts draws it in the selected framebuffer
//-------------------------------------
void R_BlitColorIntoScreen(int color, SDL_Rect* pos);

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
void R_DrawPixelShaded(int x, int y, int color, float intensity, float distance);

//-------------------------------------
// Draw a column of pixels with shading
//-------------------------------------
void R_DrawColumnOfPixelShaded(int x, int y, int endY, int color, float intensity, float distance);

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
// Calls the  the Raycast routines and draws the walls
//-------------------------------------
void R_Raycast(void);

//-------------------------------------
// Raycasts the camera's level, therefore with occlusion
//-------------------------------------
void R_RaycastPlayersLevel(int level, int x, float _rayAngle);

//-------------------------------------
// Raycasts levels above and below the camera's level, therefore without occlusion
//-------------------------------------
void R_RaycastLevelNoOcclusion(int level, int x, float _rayAngle);

//-------------------------------------
// Draws the bottom face of a cube that's located above camera's head
//-------------------------------------
void R_DrawWallBottom(walldata_t* wall, float height, float screenZ, bool isInFront);

//-------------------------------------
// Draws the bottom top of a cube that's located below camera's head
//-------------------------------------
void R_DrawWallTop(walldata_t* wall, float height, float screenZ, bool isInFront);

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
void R_FloorCasting(int end, float rayAngle, int x, float wallHeight);

//-------------------------------------
// Floorcast and ceilingcast
// Params:
// - end = the end of the wall that states where to start to floorcast
// - rayAngle = the current rayangle
// - x = the x coordinate on the screen for this specific floor cast call
//-------------------------------------
void R_CeilingCasting(int level, float start, float rayAngle, int x, float wallHeight);

//-------------------------------------
// Adds a sprite to the visible sprite array and adds its corresponding drawable
//-------------------------------------
void R_AddToVisibleSprite(int gridX, int gridY, int level, int spriteID);

//-------------------------------------
// Adds a dynamic sprite to the visible sprite array and adds its corresponding drawable
//-------------------------------------
void R_AddDynamicToVisibleSprite(int level, int gridX, int gridY);

//-------------------------------------
// Adds a dynamic sprite to the visible sprite array and adds its corresponding drawable
//-------------------------------------
void R_AddDeadDynamicToVisibleSprite(int level, int gridX, int gridY);

//-------------------------------------
// Draws the visible sprites
//-------------------------------------
void R_DrawSprite(sprite_t* sprite);

//-------------------------------------
// Draws the visible sprites
//-------------------------------------
void R_DrawDynamicSprite(dynamicSprite_t* sprite);

//-------------------------------------
// Given a level and the grid coordinates, returns what is in the map
//-------------------------------------
wallObject_t* R_GetWallObjectFromMap(int level, int y, int x);

//-------------------------------------
// Given a level and the grid coordinates, returns what is in the map
//-------------------------------------
int R_GetValueFromSpritesMap(int level, int y, int x);

void R_SetValueFromSpritesMap(int level, int y, int x, int value);

void R_SetValueFromCollisionMap(int level, int y, int x, int value);

//-------------------------------------
// Draws a column of pixels with texture mapping
//-------------------------------------
void R_DrawColumnTextured(int x, int y, int endY, SDL_Surface* texture, int xOffset, float wallheight);

//-------------------------------------
// Draws a column of pixels with texture mapping and shading
//-------------------------------------
void R_DrawStripeTexturedShaded(int x, int y, int endY, SDL_Surface* texture, int xOffset, float wallheight, float intensity, float dist);

void R_QueueAlertMessage(alertMessage_t* m, int x, int y, char* msg, float duration, float size);
void R_UpdateAlertMessages(void);

#endif