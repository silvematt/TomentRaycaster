#include "G_Physics.h"

// Timer needed to calculate delta time
Timer* stepTimer;

// Delta time
float deltaTime;

//-------------------------------------
// Initializes the physics
//-------------------------------------
void G_PhysicsInit(void)
{
    stepTimer = U_TimerCreateNew();
    stepTimer->Start(stepTimer);
    deltaTime = stepTimer->GetTicks(stepTimer) / 1000.0f;
}

//-------------------------------------
// Physics tick
//-------------------------------------
void G_PhysicsTick(void)
{
    deltaTime = stepTimer->GetTicks(stepTimer) / 1000.0f;
}

//-------------------------------------
// End the physics tick
//-------------------------------------
void G_PhysicsEndTick(void)
{
    stepTimer->Start(stepTimer);
}