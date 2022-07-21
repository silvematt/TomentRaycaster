#include <stdio.h>
#include "D_AssetsManager.h"

tomentdatapack_t tomentdatapack;

//-------------------------------------
// Sets an Object 
//-------------------------------------
void D_SetObject(object_t* obj, objectID_t id, SDL_Surface* texture, object_t* alt)
{
    obj->ID = id;
    obj->texture = texture;
    obj->alt = alt;
}


//-------------------------------------
// Initializes the Assets and datapacks 
//-------------------------------------
void D_InitAssetManager(void)
{
    // WALL CAVE 1
    // Define alt
    object_t* wallCave1Alt = (object_t*)malloc(sizeof(object_t));
    D_SetObject(wallCave1Alt, W_Cave1, SDL_LoadBMP("Data/wall1alt.bmp"), NULL);

    tomentdatapack.objects[W_Cave1].ID = W_Cave1;
    tomentdatapack.objects[W_Cave1].texture = SDL_LoadBMP("Data/wall1.bmp");
    tomentdatapack.objects[W_Cave1].alt = wallCave1Alt;

    tomentdatapack.objects[W_Cave2].ID = W_Cave2;
    tomentdatapack.objects[W_Cave2].texture = SDL_LoadBMP("Data/wall2.bmp");
    tomentdatapack.objects[W_Cave2].alt = NULL;
}