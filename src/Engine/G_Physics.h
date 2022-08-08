#ifndef PHYSICS_H_INCLUDED
#define PHYSICS_H_INCLUDED

#include "G_Physics.h"
#include "U_Timer.h"

// Timer needed to calculate delta time
extern Timer* stepTimer;

// Delta time
extern float deltaTime;

//-------------------------------------
// Initializes the physics
//-------------------------------------
void G_PhysicsInit(void);

//-------------------------------------
// Physics tick
//-------------------------------------
void G_PhysicsTick(void);

//-------------------------------------
// End the physics tick
//-------------------------------------
void G_PhysicsEndTick(void);
#endif
