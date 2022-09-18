#ifndef PHYSICS_H_INCLUDED
#define PHYSICS_H_INCLUDED

#include "P_Physics.h"
#include "U_Timer.h"
#include "U_DataTypes.h"

// Timer needed to calculate delta time
extern Timer* stepTimer;

// Delta time
extern float deltaTime;

//-------------------------------------
// Initializes the physics
//-------------------------------------
void P_PhysicsInit(void);

//-------------------------------------
// Physics tick
//-------------------------------------
void P_PhysicsTick(void);

//-------------------------------------
// End the physics tick
//-------------------------------------
void P_PhysicsEndTick(void);

//-------------------------------------
// Checks the collision between two circles
//-------------------------------------
float P_CheckCircleCollision(circle_t* c1, circle_t* c2);


float P_GetDistance(float x1, float y1, float x2, float y2);

float P_GetDistanceSquared(float x1, float y1, float x2, float y2);


#endif
