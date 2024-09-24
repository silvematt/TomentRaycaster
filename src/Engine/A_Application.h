#ifndef APPLICATION_H_INCLUDED
#define APPLICATION_H_INCLUDED

#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <stdlib.h>

#include "SDL.h"
#include "SDL_thread.h"

// Defines
#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

#define MAX_STRLEN 255      // Max lenght of a string to read

#define MAX_FILEPATH_L 255  // Max lenght of a filepath
#define MAX_STRL_R 8192     // Max lenght of a fgets while reading 


// Holds the current game state
typedef enum gamestate_e
{
    dev = 0,
    GSTATE_MENU,
    GSTATE_GAME
} gamestate_e;

// Fundamental information about the application
typedef struct app_s
{
    SDL_Window* win;
    bool quit;
    gamestate_e gamestate;
} app_t;

// Declarations
extern app_t application;

extern SDL_Surface* win_surface;    // the surface of the window        
extern int win_width;               // win_surface->w
extern unsigned int* pixels;        // pixels of the surface

extern SDL_Surface* raycast_surface;    // the surface the raycasting is written to      
extern unsigned int* raycast_pixels;    // pixels of the surface

// Multithreading rendering
extern int cpuCount;            // Number of threads to spawn
extern SDL_Thread** threads;    // all threads

// Per-thread data
typedef struct thread_data_s
{
    int localID;
} thread_data_t;

typedef enum thread_job_e
{
    TS_RAYCAST = 0,
    TS_FLOORCAST,
    TS_CEILINGCAST
} thread_job_e;

extern bool** threadDone;               // array of booleans, threadDone[i] tellls if the i-th thread has finished the computation (but doesn't mean it ended the loop)
extern bool threadIterationFinished;    // if true, the current iteration is finished
extern int extingThreads;               // how many threads finished and exited the current iteration and are ready for the next

extern thread_job_e currentThreadJob;

extern SDL_mutex* threadDataMutex;          // Mutex to access and modify the data defined above (threadDone, threadIterationFinished, extingThreads)
extern SDL_cond* condThreadWait;            // When one thread is done (but not all of them), he will wait on this condition
extern SDL_sem* renderingThreadSemaphore;   // When a thread is ready to work, he will wait on this semaphore before going inside the function, when the main thread wants to start the work of the threads, he will post on this semaphore
extern SDL_sem* mainThreadWaitSem;          // When all threads are done and extingThreads = cpuCount, means the iteration is definitely finished, so we can wake up the main thread and continue

extern SDL_mutex* rayDataMutex;             // Mutex to access and modify the data filled while raycasting (AddToThinWall, AddDynamicSprites, etc)

//-------------------------------------
// Initializes the application 
//-------------------------------------
void A_InitApplication(void);

//-------------------------------------
// Quit Application 
//-------------------------------------
void A_QuitApplication(void);

//-------------------------------------
// Change State 
//-------------------------------------
void A_ChangeState(gamestate_e newState);

#endif
