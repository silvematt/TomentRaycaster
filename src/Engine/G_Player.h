#ifndef PLAYER_H_INCLUDED
#define PLAYER_H_INCLUDED

#include "A_Application.h"
#include "G_Game.h"
#include "U_DataTypes.h"
#include "D_AssetsManager.h"

// Minimap Player
#define PLAYER_WIDTH 16
#define PLAYER_HEIGHT 16

#define PLAYER_CENTER_FIX 8     // To fix the player position to be at the center instead of the upper corner, this is WIDTH/2

#define PLAYER_SPEED 250.0f
#define PLAYER_ROT_SPEED 0.5f

#define PLAYER_STARTING_ROT M_PI

// The minimum distance the player can get to the wall
#define PLAYER_MIN_DIST_TO_WALL 30

// The light that the player will always carry, no matter the current map
#define PLAYER_POINT_LIGHT_INTENSITY 10

#define PLAYER_AI_HIT_DISTANCE 110.0f

#define ATTACK_CONE_SIZE 4
#define ATTACK_CONE_MAX_DIFF 25

#define CROSSHAIR_HIT_TIME_SECONDS 0.35f

typedef enum playerState_e
{
    PSTATE_IDLE = 0,
    PSTATE_ATTACKING1,
    PSTATE_CASTSPELL
} playerState_e;

typedef enum playerAttacks_e
{
    P_ATTACK_MELEE_1 = 0,
    P_ATTACK_CAST_FIREBALL1
} playerAttacks_e;

typedef enum playerSpells_e
{
    SPELL_FIREBALL1 = 4, // Needs to be the same as the corresponding sprite that will be instantiated as the projectile
    SPELL_ICEDART1 = 8
} playerSpells_e;

// ----------------------------------------------------
// Sets an SDL_Rect
// ----------------------------------------------------
typedef struct player_s
{
    vector2_t position;     // abs SDL position
    vector2_t centeredPos;  // position centered for the player's width and height
    float z;
    int level;              // The current floor level the player is at
    
    circle_t collisionCircle;

    vector2Int_t gridPosition;  // position of the player in the tilemap
    vector2Int_t inFrontGridPosition; // The grid pos of the cell in front of the player
    vector2Int_t attackConeGridPos[ATTACK_CONE_SIZE]; // The grid pos of the cells in front of the player that represents the cone of attack, AI that will be in these cells will be checked for hit

    vector2_t deltaPos; // used to calculate movements and collision

    // Minimap rapresentaton
    SDL_Surface* surface;
    SDL_Rect surfaceRect;

    // Rot of the player in radians
    float angle;

    // Loaded at runtime
    bool hasBeenInitialized;

    // The grid the player starts in (loaded from map)
    int startingGridX; 
    int startingGridY;
    float startingRot;

    playerState_e state;
    playerFPID_e curWeapon;
    playerSpells_e curSpell;

    entityattributes_t attributes;

    // Animation
    Timer* animTimer;
    bool animPlay;
    int animFrame;
    bool animPlayOnce;
    int playedFrames;

    // Has to attack/cast a spell, used to not attack more than once while playing the animation
    bool hasToCast;
    bool hasCasted;

    // Timer used by the crosshair to change its color when the player hits an enemy
    bool crosshairHit;
    Timer* crosshairTimer;

    // Weapons
    bool hasAxe;

    // Spells
    bool hasIceDart;

} player_t;


extern player_t player;

//-------------------------------------
// Initializes Player 
//-------------------------------------
void G_InitPlayer(void);

//-------------------------------------
// Player's Tick 
//-------------------------------------
void G_PlayerTick(void);

//-------------------------------------
// Handles Input from the player while reading the keyboard state
//-------------------------------------
void G_InGameInputHandling(const uint8_t* keyboardState, SDL_Event* e);

//-------------------------------------
// Handles Input from the player while doing the Event Input Handling
//-------------------------------------
void G_InGameInputHandlingEvent(SDL_Event* e);

//-------------------------------------
// Checks the collision map at player's level and returns what found
//-------------------------------------
int G_CheckCollisionMap(int level, int y, int x);

//-------------------------------------
// Checks door state map at player's level and returns what found
//-------------------------------------
int G_GetDoorState(int level, int y, int x);

//-------------------------------------
// Sets door state map at player's level
//-------------------------------------
int G_SetDoorState(int level, int y, int x, doorstate_e state);

//-------------------------------------
// Checks door position map at player's level and returns what found
//-------------------------------------
float G_GetDoorPosition(int level, int y, int x);

//-------------------------------------
// Checks object T map at player's level and returns what found
//-------------------------------------
int G_GetFromObjectTMap(int level, int y, int x);

void G_SetObjectTMap(int level, int y, int x, int value);

void G_PlayerCollisionCheck();

void G_PlayerRender(void);

void G_PlayerUIRender(void);

void G_PlayerPlayAnimationOnce(objectanimationsID_e animID);

bool G_PlayerCanAttack(void);

void G_PlayerTakeDamage(float dmg);

void G_PlayerDrainMana(float amount);

void G_PlayerGainHealth(float amount);
void G_PlayerGainMana(float amount);


#endif