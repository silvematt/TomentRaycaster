#ifndef ASSETS_MANAGER_H_INCLUDED
#define ASSETS_MANAGER_H_INCLUDED

#include "../include/SDL2/SDL.h"
#include "U_DataTypes.h"

// --------------------------------------------
// DEFINES
// --------------------------------------------
#define OBJECTARRAY_DEFAULT_SIZE 256

// --------------------------------------------
// All Walls and doors
// 
// W_   walls
// WD_  doors
// --------------------------------------------
typedef enum wallObjectID_e
{
    // 0 = Empty
    W_1 = 1,
    W_1Alt = 2, // Alt is vertical
    W_2 = 3,
    WD_Gate1 = 4,
    WD_Gate1Alt = 5,
} wallObjectID_t;

// All Floors
typedef enum floorObjectID_e
{
    // 0 = Empty
    F_1 = 1,
} floorObjectID_t;

// All Celings
typedef enum ceilingObjectID_e
{
    // 0 = Empty
    C_1 = 1,
} ceilingObjectID_t;

// All sprites
typedef enum spritesObjectID_e
{
    // 0 = Empty
    S_Barrel1 = 1,
} spritesObjectID_t;

typedef enum doorsObjectID_e
{
    // 0 = Empty
    D_Gate1 = 1
} doorsObjectID_t;

typedef struct object_s
{
    int ID;
    SDL_Surface* texture;
    struct object_s* alt;
    byte flags;             // Flags to diversify types of objects
} object_t;

/* object_t Flags

    // ============
    // For walls
    // ============
    // 0000000 0
    //          \
    //           1 = Is Thin Wall

    // 000000 0 0
    //         \
    //          1 = Is Vertical (used with thin Wall)

    // 00000  0 00
    //         \
    //          1 = Is Door

    // ============
    // For sprites
    // ============
    For sprites
    // 0000000 0
    //          \
    //           1 = solid (used for collision checking)
*/

// The whole datapack of the game
typedef struct tomentdatapack_s
{
    // All the walls in the game
    object_t* walls[OBJECTARRAY_DEFAULT_SIZE];
    object_t* floors[OBJECTARRAY_DEFAULT_SIZE];
    object_t* ceilings[OBJECTARRAY_DEFAULT_SIZE];
    object_t* sprites[OBJECTARRAY_DEFAULT_SIZE];
} tomentdatapack_t;

    
extern tomentdatapack_t tomentdatapack;

//-------------------------------------
// Initializes defauls for an object
//-------------------------------------
void D_InitObject(object_t* obj);

void D_InitAssetManager(void);
void D_InitLoadWalls(void);
void D_InitLoadFloors(void);
void D_InitLoadCeilings(void);
void D_InitLoadSprites(void);

//-------------------------------------
// Sets the object for the given parameters
//-------------------------------------
void D_SetObject(object_t* obj, int id, SDL_Surface* texture, object_t* alt);

#endif