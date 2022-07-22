#include <stdio.h>
#include "A_Application.h"
#include "D_AssetsManager.h"

tomentdatapack_t tomentdatapack;

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
    // Create Objects
    object_t* wallCave1 = (object_t*)malloc(sizeof(object_t));
    object_t* wallCave1Alt = (object_t*)malloc(sizeof(object_t));
    object_t* wallCave2 = (object_t*)malloc(sizeof(object_t));
    object_t* floor1 = (object_t*)malloc(sizeof(object_t));
    object_t* ceiling1 = (object_t*)malloc(sizeof(object_t));

    // Put objects in the datapack
    tomentdatapack.objects[W_Cave1] = wallCave1;
    tomentdatapack.objects[W_Cave1Alt] = wallCave1Alt;
    tomentdatapack.objects[W_Cave2] = wallCave2;
    tomentdatapack.objects[W_Floor1] = floor1;
    tomentdatapack.objects[W_Ceiling1] = ceiling1;

    // Fill objects
    // Convert all the surfaces that we will load in the same format as the win_surface
    SDL_Surface *temp1;

    temp1 = SDL_LoadBMP("Data/wall1.bmp");
    tomentdatapack.objects[W_Cave1]->texture = SDL_ConvertSurface(temp1, win_surface->format, 0);
    SDL_FreeSurface(temp1);

    temp1 = SDL_LoadBMP("Data/wall1alt.bmp");
    tomentdatapack.objects[W_Cave1Alt]->texture = SDL_ConvertSurface(temp1, win_surface->format, 0);
    SDL_FreeSurface(temp1);

    temp1 = SDL_LoadBMP("Data/wall2.bmp");
    tomentdatapack.objects[W_Cave2]->texture = SDL_ConvertSurface(temp1, win_surface->format, 0);
    SDL_FreeSurface(temp1);

    temp1 = SDL_LoadBMP("Data/floor.bmp");
    tomentdatapack.objects[W_Floor1]->texture = SDL_ConvertSurface(temp1, win_surface->format, 0);
    SDL_FreeSurface(temp1);

    temp1 = SDL_LoadBMP("Data/ceiling1.bmp");
    tomentdatapack.objects[W_Ceiling1]->texture = SDL_ConvertSurface(temp1, win_surface->format, 0);
    SDL_FreeSurface(temp1);

    // Final sets
    D_SetObject(wallCave1, W_Cave1, tomentdatapack.objects[W_Cave1]->texture, wallCave1Alt);
    D_SetObject(wallCave1Alt, W_Cave1Alt, tomentdatapack.objects[W_Cave1Alt]->texture, NULL);
    D_SetObject(wallCave2, W_Cave2, tomentdatapack.objects[W_Cave2]->texture, NULL);
    D_SetObject(floor1, W_Floor1, tomentdatapack.objects[W_Floor1]->texture, NULL);
    D_SetObject(ceiling1, W_Ceiling1, tomentdatapack.objects[W_Ceiling1]->texture, NULL);
}