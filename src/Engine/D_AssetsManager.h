#ifndef ASSETS_MANAGER_H_INCLUDED
#define ASSETS_MANAGER_H_INCLUDED

#include "../include/SDL2/SDL.h"
#include "U_DataTypes.h"

// --------------------------------------------
// DEFINES
// --------------------------------------------
#define OBJECTARRAY_DEFAULT_SIZE 256

// All Walls
typedef enum wallObjectID_e
{
    // 0 = Empty
    W_1 = 1,
    W_1Alt = 2,
    W_2 = 3,
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

typedef enum spritesObjectID_e
{
    S_Barrel1 = 1,
} spritesObjectID_t;

typedef struct object_s
{
    int ID;
    SDL_Surface* texture;
    struct object_s* alt;
    byte flags; // Extra data
} object_t;

/* object_t Flags

    For sprites
    // 0000000 0
    //          \
    //           1 = solid (used for collision checking)
*/

typedef struct tomentdatapack_s
{
    // All the walls in the game
    object_t* walls[OBJECTARRAY_DEFAULT_SIZE];
    object_t* floors[OBJECTARRAY_DEFAULT_SIZE];
    object_t* ceilings[OBJECTARRAY_DEFAULT_SIZE];
    object_t* sprites[OBJECTARRAY_DEFAULT_SIZE];
} tomentdatapack_t;

    
extern tomentdatapack_t tomentdatapack;

void D_InitAssetManager(void);
void D_InitLoadWalls(void);
void D_InitLoadFloors(void);
void D_InitLoadCeilings(void);
void D_InitLoadSprites(void);

void D_SetObject(object_t* obj, int id, SDL_Surface* texture, object_t* alt);

#endif