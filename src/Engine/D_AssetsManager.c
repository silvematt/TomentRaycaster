#include <stdio.h>
#include "A_Application.h"
#include "D_AssetsManager.h"

tomentdatapack_t tomentdatapack;

//-------------------------------------
// Sets an Object 
//-------------------------------------
void D_SetObject(object_t* obj, int id, SDL_Surface* texture, object_t* alt)
{
    obj->ID = id;
    
    if(texture == NULL)
        printf("FATAL ERROR! Could not load texture with ID: %d\n", id);

    obj->texture = texture;
    obj->alt = alt;
}

//-------------------------------------
// Initializes the Assets and datapacks 
//-------------------------------------
void D_InitAssetManager(void)
{
    D_InitLoadWalls();
    D_InitLoadFloors();
    D_InitLoadCeilings();
}

void D_InitLoadWalls(void)
{
    // Create Objects
    object_t* wall1 = (object_t*)malloc(sizeof(object_t));
    object_t* wall1Alt = (object_t*)malloc(sizeof(object_t));
    object_t* wall2 = (object_t*)malloc(sizeof(object_t));

    // Put objects in the datapack
    tomentdatapack.walls[W_1] = wall1;
    tomentdatapack.walls[W_1Alt] = wall1Alt;
    tomentdatapack.walls[W_2] = wall2;

    // Fill objects
    // Convert all the surfaces that we will load in the same format as the win_surface
    SDL_Surface *temp1;

    temp1 = SDL_LoadBMP("Data/wall1.bmp");
    tomentdatapack.walls[W_1]->texture = SDL_ConvertSurface(temp1, win_surface->format, 0);
    SDL_FreeSurface(temp1);

    temp1 = SDL_LoadBMP("Data/wall1alt.bmp");
    tomentdatapack.walls[W_1Alt]->texture = SDL_ConvertSurface(temp1, win_surface->format, 0);
    SDL_FreeSurface(temp1);

    temp1 = SDL_LoadBMP("Data/wall2.bmp");
    tomentdatapack.walls[W_2]->texture = SDL_ConvertSurface(temp1, win_surface->format, 0);
    SDL_FreeSurface(temp1);

    // Final sets
    D_SetObject(wall1, W_1, tomentdatapack.walls[W_1]->texture, wall1Alt);
    D_SetObject(wall1Alt, W_1Alt, tomentdatapack.walls[W_1Alt]->texture, NULL);
    D_SetObject(wall2, W_2, tomentdatapack.walls[W_2]->texture, NULL);
}

void D_InitLoadFloors(void)
{
    // Create Objects
    object_t* floor1 = (object_t*)malloc(sizeof(object_t));

    // Put objects in the datapack
    tomentdatapack.floors[F_1] = floor1;

    // Fill objects
    // Convert all the surfaces that we will load in the same format as the win_surface
    SDL_Surface *temp1;

    temp1 = SDL_LoadBMP("Data/floor.bmp");
    tomentdatapack.floors[F_1]->texture = SDL_ConvertSurface(temp1, win_surface->format, 0);
    SDL_FreeSurface(temp1);

    // Final sets
    D_SetObject(floor1, F_1, tomentdatapack.floors[F_1]->texture, NULL);
}

void D_InitLoadCeilings(void)
{
    // Create Objects
    object_t* ceiling1 = (object_t*)malloc(sizeof(object_t));

    // Put objects in the datapack
    tomentdatapack.ceilings[C_1] = ceiling1;

    // Fill objects
    // Convert all the surfaces that we will load in the same format as the win_surface
    SDL_Surface *temp1;

    temp1 = SDL_LoadBMP("Data/ceiling1.bmp");
    tomentdatapack.ceilings[C_1]->texture = SDL_ConvertSurface(temp1, win_surface->format, 0);
    SDL_FreeSurface(temp1);

    // Final sets
    D_SetObject(ceiling1, C_1, tomentdatapack.ceilings[C_1]->texture, NULL);
}