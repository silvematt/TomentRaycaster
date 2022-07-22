#include <stdio.h>
#include "A_Application.h"
#include "D_AssetsManager.h"

tomentdatapack_t tomentdatapack;
SDL_Surface* surfaces[256];
SDL_Surface* floortext;
SDL_Surface* ceilingtext;


//-------------------------------------
// Sets an Object 
//-------------------------------------
void D_SetObject(object_t* obj, objectID_t id, SDL_Surface* texture, object_t* alt)
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
    // Used to convert all the surfaces that we will load in the same format as the win_surface
    SDL_Surface *temp1;

    temp1 = SDL_LoadBMP("Data/wall1.bmp");
    surfaces[W_Cave1] = SDL_ConvertSurface(temp1, win_surface->format, 0);
    SDL_FreeSurface(temp1);

    temp1 = SDL_LoadBMP("Data/wall1alt.bmp");
    surfaces[W_Cave2] = SDL_ConvertSurface(temp1, win_surface->format, 0);
    SDL_FreeSurface(temp1);

    temp1 = SDL_LoadBMP("Data/floor.bmp");
    surfaces[W_Floor1] = SDL_ConvertSurface(temp1, win_surface->format, 0);
    SDL_FreeSurface(temp1);

    temp1 = SDL_LoadBMP("Data/ceiling1.bmp");
    surfaces[W_Ceiling1] = SDL_ConvertSurface(temp1, win_surface->format, 0);
    SDL_FreeSurface(temp1);

    // WALL CAVE 1
    object_t* wallCave1 = (object_t*)malloc(sizeof(object_t));
    object_t* wallCave1Alt = (object_t*)malloc(sizeof(object_t));
    object_t* floor1 = (object_t*)malloc(sizeof(object_t));
    object_t* ceiling1 = (object_t*)malloc(sizeof(object_t));

    D_SetObject(wallCave1, W_Cave1, surfaces[W_Cave1], wallCave1Alt);
    D_SetObject(wallCave1Alt, W_Cave2, surfaces[W_Cave2], NULL);
    D_SetObject(floor1, W_Floor1, surfaces[W_Floor1], NULL);
    D_SetObject(ceiling1, W_Ceiling1, surfaces[W_Ceiling1], NULL);

    tomentdatapack.objects[W_Cave1] = wallCave1;
    tomentdatapack.objects[W_Cave2] = wallCave1Alt;
    tomentdatapack.objects[W_Floor1] = floor1;
    tomentdatapack.objects[W_Ceiling1] = ceiling1;
}