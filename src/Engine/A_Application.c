#include "A_Application.h"

#include "R_Rendering.h"

// Definitions
app_t application;

// Surface of the Window
SDL_Surface* win_surface;
int win_width;
unsigned int* pixels;

// Surface of the rendered raycasting image, scaled appropiately
SDL_Surface* raycast_surface;
unsigned int* raycast_pixels;

// Game State
bool isInMenu;       // True if the player is in a menu
bool isInGame;       // True if the player is in game

// Look at .h for explaination of data
int cpuCount;
SDL_Thread** threads;
thread_data_t** threadsdata;
bool** threadDone;
int extingThreads = 0;
bool threadIterationFinished;

thread_job_e currentThreadJob = TS_RAYCAST;

SDL_mutex* threadDataMutex;
SDL_cond* condThreadWait;
SDL_sem* renderingThreadSemaphore;
SDL_sem* mainThreadWaitSem;
SDL_sem* renderingThreadSemaphore;

SDL_mutex* rayDataMutex;

//-------------------------------------
// Initializes the application and subsystems
//-------------------------------------
void A_InitApplication(void)
{
    application.quit = false;

    printf("Booting up...\n");

    SDL_Init(SDL_INIT_EVERYTHING);

    uint32_t winFlags = 0; 
    //uint32_t winFlags = SDL_WINDOW_FULLSCREEN; 

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");

    application.win = SDL_CreateWindow("Toment", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, winFlags);
    win_surface = SDL_GetWindowSurface(application.win);

    win_width = win_surface->w;
    pixels = win_surface->pixels;

    // Initialize CPU Count
    cpuCount = SDL_GetCPUCount();
    
    printf("CPU Count: %d\n", cpuCount);

    printf("Creating multithread structures...\n");
    threadIterationFinished = false;

    mainThreadWaitSem = SDL_CreateSemaphore(0);
    renderingThreadSemaphore = SDL_CreateSemaphore(0);
    threadDataMutex = SDL_CreateMutex();
    condThreadWait = SDL_CreateCond();

    rayDataMutex = SDL_CreateMutex();

    printf("Creating threads...\n");
    threads = malloc(cpuCount * sizeof(SDL_Thread*));
    threadsdata = malloc(cpuCount * sizeof(thread_data_t*));
    threadDone = malloc(cpuCount * sizeof(bool*));

    for(int i = 0; i < cpuCount; i++)
    {
        threadsdata[i] = malloc(sizeof(thread_data_t));
        threadsdata[i]->localID = i;

        threadDone[i] = malloc(sizeof(bool));

        threads[i] = SDL_CreateThread(R_ThreadRoutine, "THREAD", threadsdata[i]);
    }

    currentThreadJob = TS_RAYCAST;

    // Init Renderer
    R_InitRendering();

    srand(time(NULL));

    // Define entry state for the application
    A_ChangeState(GSTATE_MENU);
}

//-------------------------------------
// Quit Applicaiton 
//-------------------------------------
void A_QuitApplication(void)
{
    SDL_Quit();
}

//-------------------------------------
// Change State 
//-------------------------------------
void A_ChangeState(gamestate_e newState)
{
    application.gamestate = newState;

    switch(application.gamestate)
    {
        case GSTATE_MENU:
            SDL_SetRelativeMouseMode(SDL_FALSE);
            break;

        case GSTATE_GAME:
            SDL_SetRelativeMouseMode(SDL_TRUE);
            break;

        default:
            SDL_SetRelativeMouseMode(SDL_FALSE);
            break;
    }
}