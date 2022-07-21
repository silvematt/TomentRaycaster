#include "G_Physics.h"

Timer* stepTimer;
float deltaTime;

void G_PhysicsInit(void)
{
    stepTimer = U_TimerCreateNew();
    stepTimer->Start(stepTimer);
    deltaTime = stepTimer->GetTicks(stepTimer) / 1000.0f;
}

void G_PhysicsTick(void)
{
    deltaTime = stepTimer->GetTicks(stepTimer) / 1000.0f;
}

void G_PhysicsEndTick(void)
{
    stepTimer->Start(stepTimer);
}