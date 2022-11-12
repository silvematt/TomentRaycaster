#ifndef OBJECTS_CALLBACKS_H_INCLUDED
#define OBJECTS_CALLBACKS_H_INCLUDED

//-------------------------------------
// Changes the map to the parameter
//-------------------------------------
void D_CallbackChangeMap(char* data);

//-------------------------------------
// Picks the passed object
//-------------------------------------
void D_CallbackPickup(char* data);

//-------------------------------------
// Climbs the ladder up
//-------------------------------------
void D_CallbackLadder(char* data);

//-------------------------------------
// Climbs the ladder down
//-------------------------------------
void D_CallbackLadderDown(char* data);

//-------------------------------------
// Activates an altar
//-------------------------------------
void D_CallbackUseAltar(char* data);

#endif
