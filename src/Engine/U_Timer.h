// Header Guard
#ifndef TIMER_H_INCLUDED
#define TIMER_H_INCLUDED

#include <stdio.h>
#include <stdbool.h>

#include "../include/SDL2/SDL.h"

// -------------------------------
// Timer
// -------------------------------
typedef struct Timer
{
    unsigned int timerTicks;
    unsigned int pausedTicks;
    bool isPaused;
    bool isStarted;

    void (*Init)(struct Timer* this);

    void (*Start)(struct Timer* this);
    void (*Stop)(struct Timer* this);
    void (*Pause)(struct Timer* this);
    void (*Resume)(struct Timer* this);

    unsigned int (*GetTicks)(struct Timer* this);

    bool (*IsStarted)(struct Timer* this);
    bool (*IsPaused)(struct Timer* this);


} Timer;

// -------------------------------
// Allocates and creates a Timer
// -------------------------------
Timer* U_TimerCreateNew(void);

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

#endif