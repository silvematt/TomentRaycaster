#ifndef DATA_TYPES_H_INCLUDED
#define DATA_TYPES_H_INCLUDED


#include <stdint.h>
#include <stdbool.h>

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

// -------------------------------
// Sprite data structure
// -------------------------------
typedef struct sprite_s
{
    vector2_t pos;          // position in world
    vector2Int_t gridPos;   // position in grid
    vector2_t pSpacePos;    // position in player space        

    int level; // which level this sprite is in

    int spriteID;
    int sheetLength;        // For Animated objects

    float dist;     // distance from player
    float height;   // how big the sprite will be drawn

} sprite_t;

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
} drawabletype_e;

// -------------------------------
// Drawables are object in the world that will need to be drawn
// A drawable array holds everything to render and allows Zdepth
// -------------------------------
typedef struct drawabledata_s 
{
    unsigned type;

    // Pointers
    walldata_t* wallPtr;
    sprite_t* spritePtr;

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