#ifndef ASSETS_MANAGER_H_INCLUDED
#define ASSETS_MANAGER_H_INCLUDED

#include "stdio.h"

#include "../include/SDL2/SDL.h"
#include "U_DataTypes.h"

// --------------------------------------------
// DEFINES
// --------------------------------------------
#define OBJECTARRAY_DEFAULT_SIZE 256
#define MAX_TOC_LENGTH 256

// All Floors
typedef enum enginesDefaultsID_e
{
    EDEFAULT_1 = 0
} enginesDefaultsID_t;

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
    WT_CastleDoorsLvl2,

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
    S_Campfire,
    DS_Skeleton,
    S_Fireball1,
    S_PickupAxe,
    S_PickupHealthPotion,
    S_PickupManaPotion,
    S_IceDart1,
    S_TomeFireball1,
    S_TomeIceDart1
} spritesObjectID_t;

// All sprites
typedef enum skiesObjectID_e
{
    // 0 = Empty
    SKY_Default1 = 1
} skiesObjectID_e;

typedef enum objectanimationsID_e
{
    ANIM_IDLE = 0,
    ANIM_ATTACK1,
    ANIM_DIE,
    ANIM_CAST_SPELL
} objectanimationsID_e;

// Specific object data for AI
typedef struct objectAnimations_s
{
    struct object_s* belongsTo;
    
    SDL_Surface* animIdle;
    unsigned animIdleSheetLength;
    unsigned animIdleActionFrame;

    SDL_Surface* animDie;
    unsigned animDieSheetLength;
    unsigned animDieActionFrame;

    SDL_Surface* animAttack;
    unsigned animAttackSheetLength;
    unsigned animAttackActionFrame;

    SDL_Surface* animCastSpell;
    unsigned animCastSpellSheetLength;
    unsigned animCastSpellActionFrame;
} objectanimations_t;

typedef struct object_s
{
    int ID;
    SDL_Surface* texture;
    struct object_s* alt;
    byte flags;             // Flags to diversify types of objects

    objectanimations_t* animations;

    // Extra textures, after being init they point to texture
    SDL_Surface** topTexture;
    SDL_Surface** bottomTexture;

    // Extra
    char* data;
    void (*Callback)(char* data);
} object_t;


// The Text rendering is not hardcoded to use 16x6 elements font sheets, but the translation map is, 
// if you wish to use more character or a different map of the characters, you'll have to edit the translation code, 
// but the system's code should work just fine
#define FONT_MAX_ELEMENTS_WIDTH 16
#define FONT_MAX_ELEMENTS_HEIGHT 6

typedef struct fontSheet_s
{
    unsigned int width;             // Width of the tiles that compose the font sheet
    int numHorElements;             // How many horizontal items this sheet has
    int numVerElements;             // How many vertical items this sheet has
    SDL_Surface* texture;           // The whole fontsheet as an SDL_Surface
    int glyphsWidth[FONT_MAX_ELEMENTS_HEIGHT][FONT_MAX_ELEMENTS_WIDTH];        // The actual used width of each glyph, used for text-spacing
} fontsheet_t;

typedef struct uiAssets_s
{
    SDL_Surface* texture;
} uiAssets_t;

// All Fonts IDS
typedef enum fontsID_e
{
    FONT_BLKCRY = 0,
} fontsID_t;

// Menu Assets IDS
typedef enum uiAssetsID_e
{
    M_ASSET_SELECT_CURSOR = 0,
    M_ASSET_TITLE,
    G_ASSET_HEALTHBAR_EMPTY,
    G_ASSET_HEALTHBAR_FILL,
    G_ASSET_MANABAR_EMPTY,
    G_ASSET_MANABAR_FILL,
    G_ASSET_ICON_FISTS,
    G_ASSET_ICON_AXE,
    G_ASSET_ICON_SPELL_FIREBALL1,
    G_ASSET_UI_CROSSHAIR,
    G_ASSET_ICON_SPELL_ICEDART1,
    G_ASSET_UI_CROSSHAIR_HIT,
} uiAssetsID_e;

// FP Player IDs
typedef enum playerFPID_e
{
    PLAYER_FP_HANDS = 0,
    PLAYER_FP_AXE
} playerFPID_e;


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

    // 0000   0   000
    //         \
    //          1 = Is Teleporter

    // ============
    // For sprites
    // ============
    For sprites
    // 0000000 0
    //          \
    //           1 = solid (used for collision checking)

    // 000000 0 0
    //         \
    //          1 = Animated sprite (uses horizontal sheet)

    // 00000  0 00
    //         \
    //          1 = Dynamic sprite: A dynamic sprite is a sprite that can move in the world and update the sprites maps, used for AI

        // 0000 0  000
    //          \
    //           1 = Auto call callback (if present) upon player's collision
*/

// Table of Content elements for opening archives (MUST BE IN SYNCH WITH ARCH)
typedef struct tocElement_s
{
    uint32_t id;
    uint32_t startingOffset;
    uint32_t size;
} tocElement_t;

// IDs of the Images in the IMGArchive (MUST BE IN SYNCH WITH ARCH)
typedef enum imgIDs_e
{
    IMG_ID_EDEFAULT_1 = 0,
    IMG_ID_W_1,
    IMG_ID_W_1Alt,
    IMG_ID_W_2,
    IMG_ID_WD_Gate1,
    IMG_ID_WD_Gate1Alt,
    IMG_ID_F_1,
    IMG_ID_C_1,
    IMG_ID_S_Barrel1,
    IMG_ID_S_Campfire,
    IMG_ID_BLKCRY_TEXT_SHEET,
    IMG_ID_MENU_SELECT_CURSOR,
    IMG_ID_MENU_TITLE,
    IMG_ID_SKY_DEFAULT,
    IMG_ID_P_HANDS_IDLE,
    IMG_ID_WT_CASTLE_DOORS,
    IMG_ID_AI_SKELETON,
    IMG_ID_AI_SKELETON_DEATH,
    IMG_ID_P_HANDS_ATTACK,
    IMG_ID_P_HANDS_CASTSPELL,
    IMG_ID_SPELL_FIREBALL1,
    IMG_ID_SPELL_FIREBALL1_EXPLOSION,
    IMG_ID_HEALTHBAR_EMPTY,
    IMG_ID_HEALTHBAR_FILL,
    IMG_ID_MANABAR_EMPTY,
    IMG_ID_MANABAR_FILL,
    IMG_ID_AI_SKELETON_ATTACK,
    IMG_ID_PICKUP_AXE,
    IMG_ID_PICKUP_HEALTH_POTION,
    IMG_ID_PICKUP_MANA_POTION,
    IMG_ID_P_AXE_IDLE,
    IMG_ID_P_AXE_ATTACK1,
    IMG_ID_ICON_FISTS,
    IMG_ID_ICON_AXE,
    IMG_ID_ICON_SPELL_FIREBALL1,
    IMG_ID_UI_CROSSHAIR,
    IMG_ID_SPELL_ICEDART1,
    IMG_ID_SPELL_ICEDART_EXPLOSION,
    IMG_ID_ICON_SPELL_ICEDART1,
    IMG_ID_UI_CROSSHAIR_HIT,
    IMG_ID_TOME_FIREBALL01,
    IMG_ID_TOME_ICEDART01
} imgIDs_e;

typedef struct archt_s
{
    FILE* file;
    uint32_t fileLength;
    tocElement_t toc[MAX_TOC_LENGTH];
    uint32_t tocSize;
    uint32_t tocElementsLenght;
    uint32_t tocOffset; // how many bytes (fileLength included) to skip the ToC and access the data
    byte* buffer;
} archt_t;

// The whole datapack of the game
typedef struct tomentdatapack_s
{
    // -------------------------------
    // Archives
    // -------------------------------

    // img.archt
    archt_t IMGArch;

    // Font databse
    fontsheet_t fontsheets[OBJECTARRAY_DEFAULT_SIZE];   // All fonts
    unsigned fontsheetsLenghth;

    // -------------------------------
    // UI
    // -------------------------------
    uiAssets_t uiAssets[OBJECTARRAY_DEFAULT_SIZE];

    // -------------------------------
    // In Game Assets
    // -------------------------------

    // Default Engine's Objects
    object_t* enginesDefaults[OBJECTARRAY_DEFAULT_SIZE];
    unsigned enginesDefaultsLength;

    // Object in the game
    object_t* skies[OBJECTARRAY_DEFAULT_SIZE];
    unsigned skiesLength;

    object_t* walls[OBJECTARRAY_DEFAULT_SIZE];
    unsigned wallsLength;

    object_t* floors[OBJECTARRAY_DEFAULT_SIZE];
    unsigned floorsLength;

    object_t* ceilings[OBJECTARRAY_DEFAULT_SIZE];
    unsigned ceilingsLength;

    object_t* sprites[OBJECTARRAY_DEFAULT_SIZE];
    unsigned spritesLength;

    // Contains the value of the length of the spreadsheet for each sprite delcared
    // Access by spritesObjectID_e
    int spritesSheetsLenghtTable[OBJECTARRAY_DEFAULT_SIZE];

    object_t* playersFP[OBJECTARRAY_DEFAULT_SIZE];
    unsigned playersFPLength;
} tomentdatapack_t;
    
extern tomentdatapack_t tomentdatapack;

//-------------------------------------
// Returns true if the texture has correctly loaded, otherwise false and an error
//-------------------------------------
bool D_CheckTextureLoaded(SDL_Surface* ptr, int ID);

//-------------------------------------
// Initializes defauls for an object
//-------------------------------------
void D_InitObject(object_t* obj);

void D_InitAssetManager(void);

//-------------------------------------
// Opens the archives to allow objects initializations
//-------------------------------------
void D_OpenArchs(void);

//-------------------------------------
// Closes the archives to and frees buffers
//-------------------------------------
void D_CloseArchs(void);

void D_InitEnginesDefaults(void);
void D_InitFontSheets(void);
void D_InitUIAssets(void);
void D_InitLoadWalls(void);
void D_InitLoadFloors(void);
void D_InitLoadCeilings(void);
void D_InitLoadSprites(void);
void D_InitLoadSkies(void);
void D_InitLoadPlayersFP(void);

//-------------------------------------
// Sets the object for the given parameters
//-------------------------------------
void D_SetObject(object_t* obj, int id, SDL_Surface* texture, object_t* alt);

#endif