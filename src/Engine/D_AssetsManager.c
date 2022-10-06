#include <stdio.h>
#include "R_Rendering.h"
#include "A_Application.h"
#include "D_AssetsManager.h"
#include "U_Utilities.h"
#include "D_ObjectsCallbacks.h"

tomentdatapack_t tomentdatapack;

//-------------------------------------
// Sets defauls for an object
//-------------------------------------
void D_InitObject(object_t* obj)
{
    obj->flags = 0;

    obj->topTexture = &obj->texture;
    obj->bottomTexture = &obj->texture;

    obj->animations = NULL;
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

    D_InitUIAssets();
    D_InitFontSheets();

    D_InitEnginesDefaults();
    D_InitLoadCeilings();
    D_InitLoadFloors();
    D_InitLoadWalls();
    D_InitLoadSprites();
    D_InitLoadSkies();

    D_InitLoadPlayersFP();

    D_CloseArchs();
}

void D_InitUIAssets(void)
{    
    // Create Objects

    // Fill objects
    // Convert all the surfaces that we will load in the same format as the win_surface
    SDL_Surface *temp1;     // Surface
    SDL_RWops* sdlWops;     // Structure to read bytes
    int offset;             // Offset in the img.archt

    // MENU_SELECT_CURSOR
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_MENU_SELECT_CURSOR].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_MENU_SELECT_CURSOR].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_MENU_SELECT_CURSOR))
        tomentdatapack.uiAssets[M_ASSET_SELECT_CURSOR].texture = SDL_ConvertSurface(temp1, win_surface->format, 0);
    else
        printf("FATAL ERROR! Engine Default \"%s\" failed to load. Further behaviour is undefined.\n", IMG_ID_EDEFAULT_1);
    SDL_SetColorKey(tomentdatapack.uiAssets[M_ASSET_SELECT_CURSOR].texture, SDL_TRUE, r_transparencyColor);    // Make transparency color for blitting
    SDL_FreeSurface(temp1);

    // MENU_TITLE
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_MENU_TITLE].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_MENU_TITLE].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_MENU_TITLE))
        tomentdatapack.uiAssets[M_ASSET_TITLE].texture = SDL_ConvertSurface(temp1, win_surface->format, 0);
    else
        printf("FATAL ERROR! Engine Default \"%s\" failed to load. Further behaviour is undefined.\n", IMG_ID_EDEFAULT_1);
    SDL_SetColorKey(tomentdatapack.uiAssets[M_ASSET_TITLE].texture, SDL_TRUE, r_transparencyColor);    // Make transparency color for blitting
    SDL_FreeSurface(temp1);

    // HEALTHBAR_EMPTY
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_HEALTHBAR_EMPTY].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_HEALTHBAR_EMPTY].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_HEALTHBAR_EMPTY))
        tomentdatapack.uiAssets[G_ASSET_HEALTHBAR_EMPTY].texture = SDL_ConvertSurface(temp1, win_surface->format, 0);
    else
        printf("FATAL ERROR! Engine Default \"%s\" failed to load. Further behaviour is undefined.\n", IMG_ID_EDEFAULT_1);
    SDL_SetColorKey(tomentdatapack.uiAssets[G_ASSET_HEALTHBAR_EMPTY].texture, SDL_TRUE, r_transparencyColor);    // Make transparency color for blitting
    SDL_FreeSurface(temp1);

    // HEALTHBAR_FILL
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_HEALTHBAR_FILL].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_HEALTHBAR_FILL].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_HEALTHBAR_FILL))
    {
        tomentdatapack.uiAssets[G_ASSET_HEALTHBAR_FILL].texture = SDL_ConvertSurface(temp1, win_surface->format, 0);
        SDL_SetColorKey(tomentdatapack.uiAssets[G_ASSET_HEALTHBAR_FILL].texture, SDL_TRUE, r_transparencyColor);    // Make transparency color for blitting
    }
    else
        printf("FATAL ERROR! Engine Default \"%s\" failed to load. Further behaviour is undefined.\n", IMG_ID_EDEFAULT_1);
    SDL_FreeSurface(temp1);

    // MANABAR_EMPTY
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_MANABAR_EMPTY].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_MANABAR_EMPTY].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_MANABAR_EMPTY))
    {
        tomentdatapack.uiAssets[G_ASSET_MANABAR_EMPTY].texture = SDL_ConvertSurface(temp1, win_surface->format, 0);
        SDL_SetColorKey(tomentdatapack.uiAssets[G_ASSET_MANABAR_EMPTY].texture, SDL_TRUE, r_transparencyColor);    // Make transparency color for blitting
    }
    else
        printf("FATAL ERROR! Engine Default \"%s\" failed to load. Further behaviour is undefined.\n", IMG_ID_EDEFAULT_1);
    SDL_FreeSurface(temp1);

    // MANABAR_FILL
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_MANABAR_FILL].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_MANABAR_FILL].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_MANABAR_FILL))
    {
        tomentdatapack.uiAssets[G_ASSET_MANABAR_FILL].texture = SDL_ConvertSurface(temp1, win_surface->format, 0);
        SDL_SetColorKey(tomentdatapack.uiAssets[G_ASSET_MANABAR_FILL].texture, SDL_TRUE, r_transparencyColor);    // Make transparency color for blitting
    }
    else
        printf("FATAL ERROR! Engine Default \"%s\" failed to load. Further behaviour is undefined.\n", IMG_ID_EDEFAULT_1);
    SDL_FreeSurface(temp1);
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
    object_t* castleDoorsToLvl2 = (object_t*)malloc(sizeof(object_t));

    tomentdatapack.wallsLength = 9; // Set length

    D_InitObject(wall1);
    D_InitObject(wall1Alt);
    D_InitObject(wall2);
    D_InitObject(gate1);
    D_InitObject(gate1Alt);
    D_InitObject(castleDoorsToLvl2);

    // Put objects in the datapack
    tomentdatapack.walls[W_1] = wall1;
    tomentdatapack.walls[W_1Alt] = wall1Alt;
    tomentdatapack.walls[W_2] = wall2;
    tomentdatapack.walls[WD_Gate1] = gate1;
    tomentdatapack.walls[WD_Gate1Alt] = gate1Alt;
    tomentdatapack.walls[WT_CastleDoorsLvl2] = castleDoorsToLvl2;

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
    {
        tomentdatapack.walls[W_1]->texture = SDL_ConvertSurface(temp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        tomentdatapack.walls[W_1]->bottomTexture = &tomentdatapack.ceilings[C_1]->texture;
    }
    else
        tomentdatapack.walls[W_1]->texture = tomentdatapack.enginesDefaults[EDEFAULT_1]->texture;
    SDL_FreeSurface(temp1);

    // W_1Alt
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_W_1Alt].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_W_1Alt].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_W_1Alt))
        tomentdatapack.walls[W_1Alt]->texture = SDL_ConvertSurface(temp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
    else
        tomentdatapack.walls[W_1Alt]->texture = tomentdatapack.enginesDefaults[EDEFAULT_1]->texture;
    SDL_FreeSurface(temp1);

    // W_2
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_W_2].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_W_2].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_W_2))
        tomentdatapack.walls[W_2]->texture = SDL_ConvertSurface(temp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
    else
        tomentdatapack.walls[W_2]->texture = tomentdatapack.enginesDefaults[EDEFAULT_1]->texture;
    SDL_FreeSurface(temp1);

    // WD_Gate1
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_WD_Gate1].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_WD_Gate1].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_WD_Gate1))
        tomentdatapack.walls[WD_Gate1]->texture = SDL_ConvertSurface(temp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
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
        tomentdatapack.walls[WD_Gate1Alt]->texture = SDL_ConvertSurface(temp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
    else
        tomentdatapack.walls[WD_Gate1Alt]->texture = tomentdatapack.enginesDefaults[EDEFAULT_1]->texture;
    U_SetBit(&tomentdatapack.walls[WD_Gate1Alt]->flags, 0); // Set Thin Wall bit flag to 1,
    U_SetBit(&tomentdatapack.walls[WD_Gate1Alt]->flags, 1); // Set Vertical bit flag to 1
    U_SetBit(&tomentdatapack.walls[WD_Gate1Alt]->flags, 2); // Set Door bit flag to 1
    SDL_FreeSurface(temp1);

    // WD_CastleDoorsLvl2
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_WT_CASTLE_DOORS].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_WT_CASTLE_DOORS].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_WT_CASTLE_DOORS))
        tomentdatapack.walls[WT_CastleDoorsLvl2]->texture = SDL_ConvertSurface(temp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
    else
        tomentdatapack.walls[WT_CastleDoorsLvl2]->texture = tomentdatapack.enginesDefaults[EDEFAULT_1]->texture;
    U_SetBit(&tomentdatapack.walls[WT_CastleDoorsLvl2]->flags, 0); // Set Thin Wall bit flag to 1, by not setting the next bit this is horizontal
    U_SetBit(&tomentdatapack.walls[WT_CastleDoorsLvl2]->flags, 2); // Set Door bit flag to 1
    U_SetBit(&tomentdatapack.walls[WT_CastleDoorsLvl2]->flags, 3); // Set Trigger bit flag to 1

    // Set callback and data because this is a trigger
    tomentdatapack.walls[WT_CastleDoorsLvl2]->Callback = D_CallbackChangeMap;
    tomentdatapack.walls[WT_CastleDoorsLvl2]->data = "lvl2";
    
    SDL_FreeSurface(temp1);

    // Final sets
    D_SetObject(wall1, W_1, tomentdatapack.walls[W_1]->texture, wall1Alt);
    D_SetObject(wall1Alt, W_1Alt, tomentdatapack.walls[W_1Alt]->texture, NULL);
    D_SetObject(wall2, W_2, tomentdatapack.walls[W_2]->texture, NULL);
    D_SetObject(gate1, WD_Gate1, tomentdatapack.walls[WD_Gate1]->texture, NULL);
    D_SetObject(gate1Alt, WD_Gate1Alt, tomentdatapack.walls[WD_Gate1Alt]->texture, NULL);
    D_SetObject(castleDoorsToLvl2, WT_CastleDoorsLvl2, tomentdatapack.walls[WT_CastleDoorsLvl2]->texture, NULL);
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
        tomentdatapack.floors[F_1]->texture = SDL_ConvertSurface(temp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
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
        tomentdatapack.ceilings[C_1]->texture = SDL_ConvertSurface(temp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
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
    object_t* aiSkeleton = (object_t*)malloc(sizeof(object_t));
    object_t* spellFireball1 = (object_t*)malloc(sizeof(object_t));

    tomentdatapack.spritesLength = 4; // Set length

    D_InitObject(spritesBarrel1);
    D_InitObject(spritesCampfire);
    D_InitObject(aiSkeleton);
    D_InitObject(spellFireball1);

    // Put objects in the datapack
    tomentdatapack.sprites[S_Barrel1] = spritesBarrel1;
    tomentdatapack.sprites[S_Campfire] = spritesCampfire;
    tomentdatapack.sprites[DS_Skeleton] = aiSkeleton;
    tomentdatapack.sprites[S_Fireball1] = spellFireball1;

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
        tomentdatapack.sprites[S_Barrel1]->texture = SDL_ConvertSurface(temp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
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
        tomentdatapack.sprites[S_Campfire]->texture = SDL_ConvertSurface(temp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
    else
        tomentdatapack.sprites[S_Campfire]->texture = tomentdatapack.enginesDefaults[EDEFAULT_1]->texture;
    U_SetBit(&tomentdatapack.sprites[S_Campfire]->flags, 0); // Set collision bit flag to 1
    U_SetBit(&tomentdatapack.sprites[S_Campfire]->flags, 1); // Set animated sprite bit flag to 1
    // Sprite-Specific, set the lookup table for the sprite sheets length
    tomentdatapack.spritesSheetsLenghtTable[S_Campfire] = 4;
    SDL_FreeSurface(temp1);

    // LOAD DYNAMIC
    // AI Skeleton
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_AI_SKELETON].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_AI_SKELETON].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_AI_SKELETON))
    {
        tomentdatapack.sprites[DS_Skeleton]->texture = SDL_ConvertSurface(temp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);

        // Load animations as well
        tomentdatapack.sprites[DS_Skeleton]->animations = (objectanimations_t*)malloc(sizeof(objectanimations_t));
        tomentdatapack.sprites[DS_Skeleton]->animations->belongsTo = tomentdatapack.sprites[DS_Skeleton];

        // Idle = Normal
        tomentdatapack.sprites[DS_Skeleton]->animations->animIdle = SDL_ConvertSurface(temp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        tomentdatapack.sprites[DS_Skeleton]->animations->animIdleSheetLength = 0;

        // Skeleton Death
        int animOffset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_AI_SKELETON_DEATH].startingOffset);
        SDL_RWops* animSdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+animOffset, tomentdatapack.IMGArch.toc[IMG_ID_AI_SKELETON_DEATH].size);
        SDL_Surface* animTemp1 = SDL_LoadBMP_RW(animSdlWops, SDL_TRUE);
        tomentdatapack.sprites[DS_Skeleton]->animations->animDie = SDL_ConvertSurface(animTemp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        tomentdatapack.sprites[DS_Skeleton]->animations->animDieSheetLength = 4;

        SDL_FreeSurface(animTemp1);
    }
    else
        tomentdatapack.sprites[DS_Skeleton]->texture = tomentdatapack.enginesDefaults[EDEFAULT_1]->texture;
    U_SetBit(&tomentdatapack.sprites[DS_Skeleton]->flags, 0); // Set collision bit flag to 1
    U_SetBit(&tomentdatapack.sprites[DS_Skeleton]->flags, 2); // Set dynamic bit flag to 1
    SDL_FreeSurface(temp1);

    // Spell Fireball1
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_SPELL_FIREBALL1].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_SPELL_FIREBALL1].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_SPELL_FIREBALL1))
    {
        tomentdatapack.sprites[S_Fireball1]->texture = SDL_ConvertSurface(temp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);

        // Load animations as well
        tomentdatapack.sprites[S_Fireball1]->animations = (objectanimations_t*)malloc(sizeof(objectanimations_t));
        tomentdatapack.sprites[S_Fireball1]->animations->belongsTo = tomentdatapack.sprites[S_Fireball1];

        // Idle = Normal
        tomentdatapack.sprites[S_Fireball1]->animations->animIdle = SDL_ConvertSurface(temp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        tomentdatapack.sprites[S_Fireball1]->animations->animIdleSheetLength = 4;

        // Skeleton Death
        int animOffset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_SPELL_FIREBALL1_EXPLOSION].startingOffset);
        SDL_RWops* animSdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+animOffset, tomentdatapack.IMGArch.toc[IMG_ID_SPELL_FIREBALL1_EXPLOSION].size);
        SDL_Surface* animTemp1 = SDL_LoadBMP_RW(animSdlWops, SDL_TRUE);
        tomentdatapack.sprites[S_Fireball1]->animations->animDie = SDL_ConvertSurface(animTemp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        tomentdatapack.sprites[S_Fireball1]->animations->animDieSheetLength = 4;

        SDL_FreeSurface(animTemp1);
    }
    else
        tomentdatapack.sprites[S_Fireball1]->texture = tomentdatapack.enginesDefaults[EDEFAULT_1]->texture;
    U_SetBit(&tomentdatapack.sprites[S_Fireball1]->flags, 0); // Set collision bit flag to 1
    U_SetBit(&tomentdatapack.sprites[S_Fireball1]->flags, 1); // Set animated sprite bit flag to 1
    // Sprite-Specific, set the lookup table for the sprite sheets length
    tomentdatapack.spritesSheetsLenghtTable[S_Fireball1] = 4;
    SDL_FreeSurface(temp1);


    // Final sets
    D_SetObject(spritesBarrel1, S_Barrel1, tomentdatapack.sprites[S_Barrel1]->texture, NULL);
    D_SetObject(spritesCampfire, S_Campfire, tomentdatapack.sprites[S_Campfire]->texture, NULL);
    D_SetObject(aiSkeleton, DS_Skeleton, tomentdatapack.sprites[DS_Skeleton]->texture, NULL);
}


void D_InitFontSheets(void)
{
    // Convert all the surfaces that we will load in the same format as the win_surface
    SDL_Surface *temp1;     // Surface
    SDL_RWops* sdlWops;     // Structure to read bytes
    int offset;             // Offset in the img.archt

    SDL_Surface* glyphSurface; // Contains a single glyph, used to calculate glyphWidth
    glyphSurface = SDL_CreateRGBSurfaceWithFormat(0, 32, 32, 24, win_surface->format->format);
    SDL_FillRect(glyphSurface, NULL, r_transparencyColor);

    tomentdatapack.fontsheetsLenghth = 1; // Set length

    // Load BLCKCRY font
    int width = 32;
    int nHorElements = 16;
    int nVerElements = 6;
    tomentdatapack.fontsheets[FONT_BLKCRY].width = width;
    tomentdatapack.fontsheets[FONT_BLKCRY].numHorElements = nHorElements;
    tomentdatapack.fontsheets[FONT_BLKCRY].numVerElements = nVerElements;

    // Load sheet and put it into texture
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_BLKCRY_TEXT_SHEET].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_BLKCRY_TEXT_SHEET].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_BLKCRY_TEXT_SHEET))
    {
        tomentdatapack.fontsheets[FONT_BLKCRY].texture = SDL_ConvertSurface(temp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        SDL_SetColorKey(tomentdatapack.fontsheets[FONT_BLKCRY].texture, SDL_TRUE, r_transparencyColor);    // Make transparency color for blitting
    }
    else
        tomentdatapack.fontsheets[FONT_BLKCRY].texture = tomentdatapack.enginesDefaults[EDEFAULT_1]->texture;
    SDL_FreeSurface(temp1);


    // Calculate Glyph Width (used for spacing)
    SDL_Rect glyphSurfaceRect = {0, 0, tomentdatapack.fontsheets[FONT_BLKCRY].width, tomentdatapack.fontsheets[FONT_BLKCRY].width};
    // For each element
    for(int y = 0; y < nVerElements; y++)
    {
        for(int x = 0; x < nHorElements; x++)
            {
                // Glyph to analyze
                SDL_Rect srcRect = {x*width, y*width, width, width};
                // Put it in the glyphSurface
                SDL_BlitSurface(tomentdatapack.fontsheets[FONT_BLKCRY].texture, &srcRect, glyphSurface, NULL);

                // Analyze the glyphSurface and calculate width by reading each line of the glyph and selecting the biggest distance of pixels between the first found and last found
                int start = 0, end = 0, length = 0;
                // For each column of the current glyph element
                for(int gY = 0; gY < width; gY++)
                {
                    bool firstPixel = false;
                    // For each line calculate the width
                    for(int gX = 0; gX < width; gX++)
                        {
                            Uint32 pixel = R_GetPixelFromSurface(glyphSurface, gX, gY);
                            if(pixel != r_transparencyColor)
                            {
                                // If it's the first time we found a non-transparent pixel
                                if(firstPixel == false)
                                {
                                    start = end = gX;
                                    firstPixel = true;
                                }
                                else
                                    end = gX;
                            }

                            // Length is calculated on based on distance, +1 because we're counting the pixels 
                            if((end - start) != 0 && length < (end - start)+1)
                                length = (end - start)+1;

                            // Save length in 
                            tomentdatapack.fontsheets[FONT_BLKCRY].glyphsWidth[y][x] = length+1;   // +1 because for this font it looks better if each character is given an extra space
                        }                    
                }

                // Override length of first character, it is always the space
                tomentdatapack.fontsheets[FONT_BLKCRY].glyphsWidth[0][0] = width/4;

                // Reset the glyphSurface
                SDL_FillRect(glyphSurface, NULL, r_transparencyColor);
            }
    }
    SDL_FreeSurface(glyphSurface);
}


void D_InitLoadSkies(void)
{
    // Create Objects
    object_t* skyDefault = (object_t*)malloc(sizeof(object_t));
    tomentdatapack.skiesLength = 1; // Set length

    D_InitObject(skyDefault);

    // Put objects in the datapack
    tomentdatapack.skies[SKY_Default1] = skyDefault;

    // Fill objects
    // Convert all the surfaces that we will load in the same format as the win_surface
    SDL_Surface *temp1;     // Surface
    SDL_RWops* sdlWops;     // Structure to read bytes
    int offset;             // Offset in the img.archt

    // Floor 1
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_SKY_DEFAULT].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_SKY_DEFAULT].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_SKY_DEFAULT))
        tomentdatapack.skies[SKY_Default1]->texture = SDL_ConvertSurface(temp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
    else
        tomentdatapack.skies[SKY_Default1]->texture = tomentdatapack.enginesDefaults[EDEFAULT_1]->texture;
    SDL_FreeSurface(temp1);

    // Final sets
    D_SetObject(skyDefault, SKY_Default1, tomentdatapack.skies[SKY_Default1]->texture, NULL);
}

void D_InitLoadPlayersFP(void)
{
    // Create Objects
    object_t* playerFPHandsIdle = (object_t*)malloc(sizeof(object_t));

    tomentdatapack.playersFPLength = 1; // Set length

    D_InitObject(playerFPHandsIdle);

    // Put objects in the datapack
    tomentdatapack.playersFP[PLAYER_FP_HANDS] = playerFPHandsIdle;

    // Fill objects
    // Convert all the surfaces that we will load in the same format as the win_surface
    SDL_Surface *temp1;     // Surface
    SDL_RWops* sdlWops;     // Structure to read bytes
    int offset;             // Offset in the img.archt

    // FP Hands
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_P_HANDS_IDLE].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_P_HANDS_IDLE].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_P_HANDS_IDLE))
    {
        tomentdatapack.playersFP[PLAYER_FP_HANDS]->texture = SDL_ConvertSurface(temp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        SDL_SetColorKey(tomentdatapack.playersFP[PLAYER_FP_HANDS]->texture, SDL_TRUE, r_transparencyColor);    // Make transparency color for blitting

        // Load animations as well
        tomentdatapack.playersFP[PLAYER_FP_HANDS]->animations = (objectanimations_t*)malloc(sizeof(objectanimations_t));
        tomentdatapack.playersFP[PLAYER_FP_HANDS]->animations->belongsTo = tomentdatapack.playersFP[0];

        // Idle = Normal
        tomentdatapack.playersFP[PLAYER_FP_HANDS]->animations->animIdle = SDL_ConvertSurface(temp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        tomentdatapack.playersFP[PLAYER_FP_HANDS]->animations->animIdleSheetLength = 0;

        SDL_SetColorKey(tomentdatapack.playersFP[PLAYER_FP_HANDS]->animations->animIdle, SDL_TRUE, r_transparencyColor);    // Make transparency color for blitting

        // Attack
        int animOffset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_P_HANDS_ATTACK].startingOffset);
        SDL_RWops* animSdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+animOffset, tomentdatapack.IMGArch.toc[IMG_ID_P_HANDS_ATTACK].size);
        SDL_Surface* animTemp1 = SDL_LoadBMP_RW(animSdlWops, SDL_TRUE);
        tomentdatapack.playersFP[PLAYER_FP_HANDS]->animations->animAttack = SDL_ConvertSurface(animTemp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        tomentdatapack.playersFP[PLAYER_FP_HANDS]->animations->animAttackSheetLength = 4;
        SDL_SetColorKey(tomentdatapack.playersFP[PLAYER_FP_HANDS]->animations->animAttack, SDL_TRUE, r_transparencyColor);    // Make transparency color for blitting
        SDL_FreeSurface(animTemp1);

        // Cast Spell
        animOffset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_P_HANDS_CASTSPELL].startingOffset);
        animSdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+animOffset, tomentdatapack.IMGArch.toc[IMG_ID_P_HANDS_CASTSPELL].size);
        animTemp1 = SDL_LoadBMP_RW(animSdlWops, SDL_TRUE);
        tomentdatapack.playersFP[PLAYER_FP_HANDS]->animations->animCastSpell = SDL_ConvertSurface(animTemp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        tomentdatapack.playersFP[PLAYER_FP_HANDS]->animations->animCastSpellSheetLength = 6;
        SDL_SetColorKey(tomentdatapack.playersFP[PLAYER_FP_HANDS]->animations->animCastSpell, SDL_TRUE, r_transparencyColor);    // Make transparency color for blitting
        SDL_FreeSurface(animTemp1);

    }
    else
        tomentdatapack.playersFP[PLAYER_FP_HANDS]->texture = tomentdatapack.enginesDefaults[EDEFAULT_1]->texture;
    SDL_FreeSurface(temp1);

    // Final sets
    D_SetObject(playerFPHandsIdle, PLAYER_FP_HANDS, tomentdatapack.playersFP[PLAYER_FP_HANDS]->texture, NULL);

}

//-------------------------------------
// Closes the archives to and frees buffers
//-------------------------------------
void D_CloseArchs(void)
{
    fclose(tomentdatapack.IMGArch.file);
    free(tomentdatapack.IMGArch.buffer);
}
