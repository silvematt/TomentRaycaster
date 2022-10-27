#ifndef TEXT_RENDERING_H_INCLUDED
#define TEXT_RENDERING_H_INCLUDED

#include "SDL.h"

// ----------------------------------------------------------------------
// Displays the given text
// ----------------------------------------------------------------------
void T_DisplayText(int fontID, char* text, int x, int y);

// ----------------------------------------------------------------------
// Displays the given text scaled by the scale factor
// ----------------------------------------------------------------------
void T_DisplayTextScaled(int fontID, char* text, int x, int y, float scaleFactor);

// ----------------------------------------------------------------------
// Given a char, returns the sprite sheet coords, most naive approach ever
//
// This translation also kills the universality of the Text Renderer by forcing the font sheet to be 16x6
// You could make different translations for each map or find a better way to translate
// ----------------------------------------------------------------------
void T_TranslateASCIIToSpriteSheetCoords(char c, int* destX, int* destY);

#endif