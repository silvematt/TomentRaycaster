#include <math.h>

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

float P_GetDistance(float x1, float y1, float x2, float y2)
{
    float dx = x1 - x2;
    float dy = y1 - y2;

    return sqrt(dx*dx + dy*dy);
}

float P_GetDistanceSquared(float x1, float y1, float x2, float y2)
{
    float dx = x1 - x2;
    float dy = y1 - y2;

    return (dx*dx + dy*dy);
}

//-------------------------------------
// Checks the collision between two circles
//-------------------------------------
float P_CheckCircleCollision(circle_t* c1, circle_t* c2)
{
    float totalRadius = c1->r + c2->r;
    totalRadius = totalRadius*totalRadius;
    
    float dist = P_GetDistanceSquared(c1->pos.x, c1->pos.y, c2->pos.x, c2->pos.y);
    if(dist < totalRadius)
        return dist;

    return -1.0f;
}


