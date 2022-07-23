#ifndef PHYSICS_H_INCLUDED
#define PHYSICS_H_INCLUDED

#include "G_Physics.h"
#include "U_Timer.h"

extern Timer* stepTimer;
extern float deltaTime;

void G_PhysicsInit(void);
void G_PhysicsTick(void);
void G_PhysicsEndTick(void);
#endif
