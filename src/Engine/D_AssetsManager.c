#include <stdio.h>
#include "A_Application.h"
#include "D_AssetsManager.h"
#include "U_Utilities.h"

tomentdatapack_t tomentdatapack;

//-------------------------------------
// Sets defauls for an object
//-------------------------------------
void D_InitObject(object_t* obj)
{
    obj->flags = 0;
}

//-------------------------------------
// Sets an Object 
//-------------------------------------
void D_SetObject(object_t* obj, int id, SDL_Surface* texture, object_t* alt)
{
    obj->ID = id;
    
    if(texture == NULL)
        printf("FATAL ERROR! Setting Object: Could not load texture with ID: %d\n", id);

    obj->texture = texture;
    obj->alt = alt;
}

bool D_CheckTextureLoaded(SDL_Surface* ptr, int ID)
{
    if(ptr == NULL)
    {
        printf("ERROR! Could not load IMG_ID:\"%d\", the file could not exist or could be corrupted. Attempting to fall back to Engines Defaults...\n", ID);
        return false;
    }

    return true;
}

//-------------------------------------
// Opens the archives to allow objects initializations
//-------------------------------------
void D_OpenArchs(void)
{
    archt_t* curArch;

    //-------------------------------------
    // Open IMG Arch
    //-------------------------------------
    curArch = &tomentdatapack.IMGArch;
    curArch->file = fopen("Data/img.archt", "rb");

    // Get file lenght
    fseek(curArch->file, 0, SEEK_END);
    curArch->fileLength = ftell(curArch->file);
    rewind(curArch->file);

    // Read TOC size in bytes
    // First 4 bytes are TOC length
    fread(&(curArch->tocSize), sizeof(curArch->tocSize), 1, curArch->file);

    // Get number of elements
    curArch->tocElementsLenght = curArch->tocSize / sizeof(tocElement_t);

    // Calculate base offset
    curArch->tocOffset = sizeof(curArch->tocSize) + sizeof(tocElement_t) * tomentdatapack.IMGArch.tocElementsLenght;

    // Fill the ToC
    for(int i = 0; i < curArch->tocElementsLenght; i++)
    {
        fread(&curArch->toc[i].id, sizeof(curArch->toc[i].id), 1, curArch->file);
        fread(&curArch->toc[i].startingOffset, sizeof(curArch->toc[i].startingOffset), 1, curArch->file);
        fread(&curArch->toc[i].size, sizeof(curArch->toc[i].size), 1, curArch->file);
    }
    rewind(curArch->file);

    // Allocate the buffer to allow reading the file and fill it
    curArch->buffer = (byte *)malloc((curArch->fileLength) * sizeof(byte));
    fread(tomentdatapack.IMGArch.buffer, (tomentdatapack.IMGArch.fileLength), 1, tomentdatapack.IMGArch.file);
}



//-------------------------------------
// Initializes the Assets and datapacks 
//-------------------------------------
void D_InitAssetManager(void)
{
    printf("Initializing Assets Manager...\n");

    D_OpenArchs();

    D_InitEnginesDefaults();
    D_InitLoadWalls();
    D_InitLoadFloors();
    D_InitLoadCeilings();
    D_InitLoadSprites();

    D_CloseArchs();
}

void D_InitEnginesDefaults(void)
{    
    // Create Objects
    object_t* texturefallback = (object_t*)malloc(sizeof(object_t));
    tomentdatapack.enginesDefaultsLength = 1; // Set length

    D_InitObject(texturefallback);

    // Put objects in the datapack
    tomentdatapack.enginesDefaults[EDEFAULT_1] = texturefallback;

    // Fill objects
    // Convert all the surfaces that we will load in the same format as the win_surface
    SDL_Surface *temp1;     // Surface
    SDL_RWops* sdlWops;     // Structure to read bytes
    int offset;             // Offset in the img.archt

    // E_1
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_EDEFAULT_1].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_EDEFAULT_1].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_EDEFAULT_1))
        tomentdatapack.enginesDefaults[EDEFAULT_1]->texture = SDL_ConvertSurface(temp1, win_surface->format, 0);
    else
        printf("FATAL ERROR! Engine Default \"%s\" failed to load. Further behaviour is undefined.\n", IMG_ID_EDEFAULT_1);
        
    SDL_FreeSurface(temp1);

    // Final sets
    D_SetObject(texturefallback, EDEFAULT_1, tomentdatapack.enginesDefaults[EDEFAULT_1]->texture, NULL);
}

void D_InitLoadWalls(void)
{
    // Create Objects
    object_t* wall1 = (object_t*)malloc(sizeof(object_t));
    object_t* wall1Alt = (object_t*)malloc(sizeof(object_t));
    object_t* wall2 = (object_t*)malloc(sizeof(object_t));
    object_t* gate1 = (object_t*)malloc(sizeof(object_t));
    object_t* gate1Alt = (object_t*)malloc(sizeof(object_t));

    tomentdatapack.wallsLength = 8; // Set length

    D_InitObject(wall1);
    D_InitObject(wall1Alt);
    D_InitObject(wall2);
    D_InitObject(gate1);
    D_InitObject(gate1Alt);

    // Put objects in the datapack
    tomentdatapack.walls[W_1] = wall1;
    tomentdatapack.walls[W_1Alt] = wall1Alt;
    tomentdatapack.walls[W_2] = wall2;
    tomentdatapack.walls[WD_Gate1] = gate1;
    tomentdatapack.walls[WD_Gate1Alt] = gate1Alt;

    // Fill objects
    // Convert all the surfaces that we will load in the same format as the win_surface
    SDL_Surface *temp1;     // Surface
    SDL_RWops* sdlWops;     // Structure to read bytes
    int offset;             // Offset in the img.archt

    // W_1
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_W_1].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_W_1].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_W_1))
        tomentdatapack.walls[W_1]->texture = SDL_ConvertSurface(temp1, win_surface->format, 0);
    else
        tomentdatapack.walls[W_1]->texture = tomentdatapack.enginesDefaults[EDEFAULT_1]->texture;
    SDL_FreeSurface(temp1);

    // W_1Alt
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_W_1Alt].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_W_1Alt].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_W_1Alt))
        tomentdatapack.walls[W_1Alt]->texture = SDL_ConvertSurface(temp1, win_surface->format, 0);
    else
        tomentdatapack.walls[W_1Alt]->texture = tomentdatapack.enginesDefaults[EDEFAULT_1]->texture;
    SDL_FreeSurface(temp1);

    // W_2
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_W_2].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_W_2].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_W_2))
        tomentdatapack.walls[W_2]->texture = SDL_ConvertSurface(temp1, win_surface->format, 0);
    else
        tomentdatapack.walls[W_2]->texture = tomentdatapack.enginesDefaults[EDEFAULT_1]->texture;
    SDL_FreeSurface(temp1);

    // WD_Gate1
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_WD_Gate1].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_WD_Gate1].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_WD_Gate1))
        tomentdatapack.walls[WD_Gate1]->texture = SDL_ConvertSurface(temp1, win_surface->format, 0);
    else
        tomentdatapack.walls[WD_Gate1]->texture = tomentdatapack.enginesDefaults[EDEFAULT_1]->texture;
    U_SetBit(&tomentdatapack.walls[WD_Gate1]->flags, 0); // Set Thin Wall bit flag to 1, by not setting the next bit this is horizontal
    U_SetBit(&tomentdatapack.walls[WD_Gate1]->flags, 2); // Set Door bit flag to 1
    SDL_FreeSurface(temp1);

    // WD_Gate1Alt
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_WD_Gate1Alt].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_WD_Gate1Alt].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_WD_Gate1Alt))
        tomentdatapack.walls[WD_Gate1Alt]->texture = SDL_ConvertSurface(temp1, win_surface->format, 0);
    else
        tomentdatapack.walls[WD_Gate1Alt]->texture = tomentdatapack.enginesDefaults[EDEFAULT_1]->texture;
    U_SetBit(&tomentdatapack.walls[WD_Gate1Alt]->flags, 0); // Set Thin Wall bit flag to 1,
    U_SetBit(&tomentdatapack.walls[WD_Gate1Alt]->flags, 1); // Set Vertical bit flag to 1
    U_SetBit(&tomentdatapack.walls[WD_Gate1Alt]->flags, 2); // Set Door bit flag to 1
    SDL_FreeSurface(temp1);

    // Final sets
    D_SetObject(wall1, W_1, tomentdatapack.walls[W_1]->texture, wall1Alt);
    D_SetObject(wall1Alt, W_1Alt, tomentdatapack.walls[W_1Alt]->texture, NULL);
    D_SetObject(wall2, W_2, tomentdatapack.walls[W_2]->texture, NULL);
    D_SetObject(gate1, WD_Gate1, tomentdatapack.walls[WD_Gate1]->texture, NULL);
    D_SetObject(gate1Alt, WD_Gate1Alt, tomentdatapack.walls[WD_Gate1Alt]->texture, NULL);
}

void D_InitLoadFloors(void)
{
    // Create Objects
    object_t* floor1 = (object_t*)malloc(sizeof(object_t));
    tomentdatapack.floorsLength = 1; // Set length

    D_InitObject(floor1);

    // Put objects in the datapack
    tomentdatapack.floors[F_1] = floor1;

    // Fill objects
    // Convert all the surfaces that we will load in the same format as the win_surface
    SDL_Surface *temp1;     // Surface
    SDL_RWops* sdlWops;     // Structure to read bytes
    int offset;             // Offset in the img.archt

    // Floor 1
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_F_1].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_F_1].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_F_1))
        tomentdatapack.floors[F_1]->texture = SDL_ConvertSurface(temp1, win_surface->format, 0);
    else
        tomentdatapack.floors[F_1]->texture = tomentdatapack.enginesDefaults[EDEFAULT_1]->texture;
    SDL_FreeSurface(temp1);

    // Final sets
    D_SetObject(floor1, F_1, tomentdatapack.floors[F_1]->texture, NULL);
}

void D_InitLoadCeilings(void)
{
    // Create Objects
    object_t* ceiling1 = (object_t*)malloc(sizeof(object_t));
    tomentdatapack.ceilingsLength = 1; // Set length

    D_InitObject(ceiling1);

    // Put objects in the datapack
    tomentdatapack.ceilings[C_1] = ceiling1;

    // Fill objects
    // Convert all the surfaces that we will load in the same format as the win_surface
    SDL_Surface *temp1;     // Surface
    SDL_RWops* sdlWops;     // Structure to read bytes
    int offset;             // Offset in the img.archt

    // Ceiling 1
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_C_1].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_C_1].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_C_1))
        tomentdatapack.ceilings[C_1]->texture = SDL_ConvertSurface(temp1, win_surface->format, 0);
    else
        tomentdatapack.ceilings[C_1]->texture = tomentdatapack.enginesDefaults[EDEFAULT_1]->texture;
    SDL_FreeSurface(temp1);

    // Final sets
    D_SetObject(ceiling1, C_1, tomentdatapack.ceilings[C_1]->texture, NULL);
}

void D_InitLoadSprites(void)
{
    // Create Objects
    object_t* spritesBarrel1 = (object_t*)malloc(sizeof(object_t));
    object_t* spritesCampfire = (object_t*)malloc(sizeof(object_t));

    tomentdatapack.spritesLength = 2; // Set length

    D_InitObject(spritesBarrel1);
    D_InitObject(spritesCampfire);

    // Put objects in the datapack
    tomentdatapack.sprites[S_Barrel1] = spritesBarrel1;
    tomentdatapack.sprites[S_Campfire] = spritesCampfire;

    // Fill objects
    // Convert all the surfaces that we will load in the same format as the win_surface
    SDL_Surface *temp1;     // Surface
    SDL_RWops* sdlWops;     // Structure to read bytes
    int offset;             // Offset in the img.archt
    
    // Barrel
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_S_Barrel1].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_S_Barrel1].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_S_Barrel1))
        tomentdatapack.sprites[S_Barrel1]->texture = SDL_ConvertSurface(temp1, win_surface->format, 0);
    else
        tomentdatapack.sprites[S_Barrel1]->texture = tomentdatapack.enginesDefaults[EDEFAULT_1]->texture;
    U_SetBit(&tomentdatapack.sprites[S_Barrel1]->flags, 0); // Set collision bit flag to 1
    // Sprite-Specific, set the lookup table for the sprite sheets length
    tomentdatapack.spritesSheetsLenghtTable[S_Barrel1] = 0;
    SDL_FreeSurface(temp1);

    // Campfire
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_S_Campfire].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_S_Campfire].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_S_Campfire))
        tomentdatapack.sprites[S_Campfire]->texture = SDL_ConvertSurface(temp1, win_surface->format, 0);
    else
        tomentdatapack.sprites[S_Campfire]->texture = tomentdatapack.enginesDefaults[EDEFAULT_1]->texture;
    U_SetBit(&tomentdatapack.sprites[S_Campfire]->flags, 0); // Set collision bit flag to 1
    U_SetBit(&tomentdatapack.sprites[S_Campfire]->flags, 1); // Set animated sprite bit flag to 1
    // Sprite-Specific, set the lookup table for the sprite sheets length
    tomentdatapack.spritesSheetsLenghtTable[S_Campfire] = 4;
    SDL_FreeSurface(temp1);


    // Final sets
    D_SetObject(spritesBarrel1, S_Barrel1, tomentdatapack.sprites[S_Barrel1]->texture, NULL);
    D_SetObject(spritesCampfire, S_Campfire, tomentdatapack.sprites[S_Campfire]->texture, NULL);
}

//-------------------------------------
// Closes the archives to and frees buffers
//-------------------------------------
void D_CloseArchs(void)
{
    fclose(tomentdatapack.IMGArch.file);
    free(tomentdatapack.IMGArch.buffer);
}
