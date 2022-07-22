#ifndef ASSETS_MANAGER_H_INCLUDED
#define ASSETS_MANAGER_H_INCLUDED

#include "../include/SDL2/SDL.h"

// --------------------------------------------
// DEFINES
// --------------------------------------------

typedef enum objectID_e
{
    // 0 = Empty
    W_Cave1 = 1,
    W_Cave2 = 2,
    W_Floor1 = 3,
    W_Ceiling1 = 4
} objectID_t;

typedef struct object_s
{
    int ID;
    SDL_Surface* texture;
    struct object_s* alt;
} object_t;


typedef struct tomentdatapack_s
{
    object_t* objects[256];
    char* maps[256];
} tomentdatapack_t;

    
extern SDL_Surface* floortext;
extern SDL_Surface* ceilingtext;

extern SDL_Surface* surfaces[256];
extern tomentdatapack_t tomentdatapack;


void D_InitAssetManager(void);
void D_SetObject(object_t* obj, objectID_t id, SDL_Surface* texture, object_t* alt);

#endif