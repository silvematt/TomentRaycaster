#ifndef DATA_TYPES_H_INCLUDED
#define DATA_TYPES_H_INCLUDED

#include <stdint.h>
#include <stdbool.h>

#include "U_Timer.h"

#define RADIAN 0.0174533        // 1 Radian


typedef	uint8_t byte;

typedef struct vector2_s
{
    float x, y;
} vector2_t;

typedef struct vector2Int_s
{
    int x,y;
} vector2Int_t;


typedef struct circle_s
{
    vector2_t pos;
    float r;
} circle_t;


//-------------------------------------
// Pathfinding
//-------------------------------------
// Node of a path
typedef struct pathnode_s
{
    vector2Int_t gridPos;

    // Costs
    int f, g, h;
    struct pathnode_s* parent;
} pathnode_t;

// Used by AI and such
typedef struct path_s
{
    bool isValid;
    pathnode_t* nodes[256];
    unsigned int nodesLength;
} path_t;

// -------------------------------
// Sprite data structure, represents static sprites
// -------------------------------
typedef struct sprite_s
{
    bool active;            // used for dynamics
    
    vector2_t pos;          // position in world
    vector2_t centeredPos;  // position in world centered
    vector2Int_t gridPos;   // position in grid
    vector2_t pSpacePos;    // position in player space        

    int level; // which level this sprite is in

    circle_t collisionCircle;

    int spriteID;
    int sheetLength;        // For Animated objects

    float dist;     // distance from player
    float height;   // how big the sprite will be drawn
} sprite_t;


typedef struct entityattributes_s
{
    float curHealth;
    float maxhealth;
} entityattributes_t;

typedef enum dynamicSpriteState_e
{
    DS_STATE_NULL = 0,
    DS_STATE_IDLE,
    DS_STATE_MOVING,
    DS_STATE_ATTACKING,
    DS_STATE_DEAD
} dynamicSpriteState_e;

// -------------------------------
// Dynamic Sprite data structure, represents dynamic sprites such as AI or projectiles
// -------------------------------
typedef struct dynamicSprite_s
{
    sprite_t base;

    // Dnyamic-Specific
    dynamicSpriteState_e state;
    bool isAlive;
    float speed;

    Timer* animTimer;
    bool animPlay;
    int animFrame;
    bool animPlayOnce;

    entityattributes_t attributes;

    vector2_t* targetPos;
    vector2Int_t* targetGridPos;
    circle_t* targetColl;

    path_t* path;
} dynamicSprite_t;

// -------------------------------
// Door states
// -------------------------------
typedef enum doorstate_e
{
    DState_Closed = 0,
    DState_Opening,
    DState_Open,
    DState_Closing
} doorstate_e;


// -------------------------------
// Holds all the info got during raycast to draw walls
// -------------------------------
typedef struct walldata_s
{
    float rayAngle;
    int x;
    float curX, curY;
    bool isVertical;
    vector2Int_t gridPos;
    float distance;
    int idHit;
    int level;
    
    // Extra Data
    // For Thin Wall Extra data is 0 = should draw this column of pixel because it is visible 1 = should not draw this column of pixel because the door is open
    int extraData;  
} walldata_t;

// -------------------------------
// Drawable Types
// -------------------------------
typedef enum drawabletype_e
{
    DRWB_WALL = 0,
    DRWB_SPRITE,
    DRWB_DYNAMIC_SPRITE
} drawabletype_e;

// -------------------------------
// Drawables are object in the world that will need to be drawn
// A drawable array holds everything to render and allows Zdepth
// -------------------------------
typedef struct drawabledata_s 
{
    drawabletype_e type;

    // Pointers
    walldata_t* wallPtr;
    sprite_t* spritePtr;
    dynamicSprite_t* dynamicSpritePtr;

    // Quick access varaibles
    float dist;
} drawabledata_t;

typedef enum orientation_e
{
    NORTH = 0,
    NORTH_WEST,
    WEST,
    SOUTH_WEST,
    SOUTH,
    SOUTH_EAST,
    EAST,
    NORTH_EAST,
    NO_ORIENTATION,
    NORTH_SOUTH,    // Specials
    WEST_EAST,
    ALL_FOUR_DIR,
} orientation_e;

#endif