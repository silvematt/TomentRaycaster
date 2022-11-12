#include "U_Timer.h"

// Timer functions
static void U_TimerInit(Timer* timer);
static void U_TimerStart(Timer* timer);
static void U_TimerStop(Timer* timer);
static void U_TimerPause(Timer* timer);
static void U_TimerResume(Timer* timer);

static unsigned int U_TimerGetTicks(Timer* timer);

// Timer checks
static bool U_TimerIsStarted(Timer* timer);
static bool U_TimerIsPaused(Timer* timer);

Timer* U_TimerCreateNew(void)
{
    Timer* timer = (Timer*)malloc(sizeof(Timer));

    timer->Init = U_TimerInit;
    timer->Start = U_TimerStart;
    timer->Stop = U_TimerStop;
    timer->Pause = U_TimerPause;
    timer->Resume = U_TimerResume;
    timer->GetTicks = U_TimerGetTicks;
    timer->IsStarted = U_TimerIsStarted;
    timer->IsPaused = U_TimerIsPaused;
    timer->Init(timer);
    
    return timer;
}

static void U_TimerInit(Timer* timer)
{
    timer->timerTicks = 0;
    timer->pausedTicks = 0;
    
    timer->isStarted = false;
    timer->isPaused = false;
}

static void U_TimerStart(Timer* timer)
{
    timer->isStarted = true;
    timer->isPaused = false;

    timer->timerTicks = SDL_GetTicks();
    timer->pausedTicks = 0;
}

static void U_TimerStop(Timer* timer)
{
    timer->timerTicks = 0;
    timer->pausedTicks = 0;
    
    timer->isStarted = false;
    timer->isPaused = false;
}

static void U_TimerPause(Timer* timer)
{
    if(timer->isStarted && !timer->isPaused)
    {
        timer->pausedTicks = SDL_GetTicks() - timer->timerTicks;
        timer->timerTicks = 0;

        timer->isPaused = true;
    }
}

static void U_TimerResume(Timer* timer)
{
    if(timer->isStarted && timer->isPaused)
    {
        timer->timerTicks = SDL_GetTicks() - timer->pausedTicks;
        timer->pausedTicks = 0;

        timer->isPaused = false;
    }
}

static unsigned int U_TimerGetTicks(Timer* timer)
{
    unsigned int time = 0;

    if(timer->isStarted)
    {
        if(timer->isPaused)
            return timer->pausedTicks;
        else
            time = SDL_GetTicks() - timer->timerTicks;
    }

    return time;
}

static bool U_TimerIsStarted(Timer* timer)
{
    return timer->isStarted;
}

static bool U_TimerIsPaused(Timer* timer)
{
    return timer->isPaused;
}