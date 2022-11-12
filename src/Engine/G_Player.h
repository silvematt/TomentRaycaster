#ifndef PLAYER_H_INCLUDED
#define PLAYER_H_INCLUDED

#include "A_Application.h"
#include "G_Game.h"
#include "U_DataTypes.h"
#include "D_AssetsManager.h"

// Minimap Player
#define PLAYER_WIDTH 16
#define PLAYER_HEIGHT 16

// To fix the player position to be at the center instead of the upper corner, this is WIDTH/2
#define PLAYER_CENTER_FIX 8     

// Speed of the player
#define PLAYER_SPEED 250.0f

// Rotation speed of the player
#define PLAYER_ROT_SPEED 0.5f

// Speed of the vertical head motion
#define PLAYER_VERTICAL_HEAD_MOVEMENT_SPEED 60.0f

// Default Player's starting rotation
#define PLAYER_STARTING_ROT M_PI

// The minimum distance the player can get to the wall
#define PLAYER_MIN_DIST_TO_WALL 30

// The light that the player will always carry, no matter the current map
#define PLAYER_POINT_LIGHT_INTENSITY 10

// The attack cone (in grids) of the player
#define ATTACK_CONE_SIZE 7

// The Max thresold of difference of angle (used to calculate if the AI we're trying to attack is in front of us)
#define ATTACK_CONE_MAX_DIFF 25

// Amount of seconds the crosshair will turn red when the player hits an AI
#define CROSSHAIR_HIT_TIME_SECONDS 0.35f

// Climbing speed
#define PLAYER_CLIMBING_LADDER_UP_SPEED 24
#define PLAYER_CLIMBING_LADDER_DOWN_SPEED 48

// State of the player
typedef enum playerState_e
{
    PSTATE_IDLE = 0,
    PSTATE_ATTACKING1,
    PSTATE_CASTSPELL,
    PSTATE_CLIMBING_LADDER
} playerState_e;

// Attacks the player can perform
typedef enum playerAttacks_e
{
    P_ATTACK_MELEE_1 = 0,
    P_ATTACK_CAST_FIREBALL1
} playerAttacks_e;

// Spells (shared with AI)
typedef enum playerSpells_e
{
    SPELL_NULL = 0,
    SPELL_FIREBALL1 = 4, // Needs to be the same as the corresponding sprite that will be instantiated as the projectile
    SPELL_ICEDART1 = 8
} playerSpells_e;

typedef struct player_s
{
    vector2_t position;     // abs SDL position
    vector2_t centeredPos;  // position centered for the player's width and height
    float z;                // z pos
    int level;              // The current floor level the player is at
    
    circle_t collisionCircle;

    vector2Int_t gridPosition;  // position of the player in the tilemap
    vector2Int_t inFrontGridPosition; // The grid pos of the cell in front of the player
    vector2Int_t attackConeGridPos[ATTACK_CONE_SIZE]; // The grid pos of the cells in front of the player that represents the cone of attack, AI that will be in these cells will be checked for hit

    vector2_t deltaPos; // used to calculate movements and collision

    // Minimap rapresentaton of the player
    SDL_Surface* surface;
    SDL_Rect surfaceRect;

    // Rot of the player in radians
    float angle;
    float verticalHeadMovement;

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
    float weaponDistance;
    bool hasAxe;
    bool hasGreatsword;

    // Spells
    bool hasFireball;
    bool hasIceDart;

    // Ladder
    bool hasToClimb;
    bool climbingUp;

    float climbingPosX;
    float climbingPosY;
    float climbingPosZ;

    bool isFightingBoss;
    dynamicSprite_t* bossFighting;
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
void G_InGameInputHandling(const uint8_t* keyboardState);

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

//-------------------------------------
// Sets a value of the object T map
//-------------------------------------
void G_SetObjectTMap(int level, int y, int x, int value);

//-------------------------------------
// Checks players collisions
//-------------------------------------
void G_PlayerCollisionCheck();

//-------------------------------------
// Renders the player
//-------------------------------------
void G_PlayerRender(void);

//-------------------------------------
// Renders the player's UI
//-------------------------------------
void G_PlayerUIRender(void);

//-------------------------------------
// Plays an animation once (FP hands)
//-------------------------------------
void G_PlayerPlayAnimationOnce(objectanimationsID_e animID);

//-------------------------------------
// Returns if the player can attack
//-------------------------------------
bool G_PlayerCanAttack(void);

//-------------------------------------
// Makes the player take damage
//-------------------------------------
void G_PlayerTakeDamage(float dmg);

//-------------------------------------
// Drains player's mana
//-------------------------------------
void G_PlayerDrainMana(float amount);

//-------------------------------------
// Adds player's health
//-------------------------------------
void G_PlayerGainHealth(float amount);

//-------------------------------------
// Adds player's mana
//-------------------------------------
void G_PlayerGainMana(float amount);

//-------------------------------------
// Sets Player's Weapon
//-------------------------------------
void G_PlayerSetWeapon(playerFPID_e weaponID);

//-------------------------------------
// Sets Player's Spell
//-------------------------------------
void G_PlayerSetSpell(playerSpells_e spellID);

//-------------------------------------
// Makes player die
//-------------------------------------
void G_PlayerDeath();

#endif