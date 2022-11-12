// Header Guard
#ifndef TIMER_H_INCLUDED
#define TIMER_H_INCLUDED

#include <stdio.h>
#include <stdbool.h>

#include "SDL.h"

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

#endif