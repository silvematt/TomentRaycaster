#include "T_TextRendering.h"
#include "D_AssetsManager.h"
#include "R_Rendering.h"

// ----------------------------------------------------------------------
// Displays the given text
// ----------------------------------------------------------------------
void T_DisplayText(int fontID, char* text, int x, int y)
{
    // The X we're currently drawing to
    int curDrawingX = x;

    // Current char in the text
    int curCIndex = 0;
    char curC = 0;

    // Width of the font
    int width = tomentdatapack.fontsheets[fontID].width;

    int curY = y; // y can be affected by \n

    // While we're not finished drawing
    while(text[curCIndex] != '\0')
    {
        if(text[curCIndex] == '\n')
        {
            curY += width;
            curDrawingX = x;
            curCIndex++;
        }
        else
        {
            // Get the char we have to draw
            curC = text[curCIndex];

            // Translate to the sprite sheet coords to get the correct texture
            int texX, texY;
            T_TranslateASCIIToSpriteSheetCoords(curC, &texX, &texY);

            // Blit it on the screen
            SDL_Rect screenPos = {curDrawingX, curY, width, width};
            SDL_Rect size = {(texX * width), (texY * width), width, width};
            R_BlitIntoScreen(&size, tomentdatapack.fontsheets[fontID].texture, &screenPos);

            // Next
            curCIndex++;
            curDrawingX += tomentdatapack.fontsheets[fontID].glyphsWidth[texY][texX];
        }
    }
}

// ----------------------------------------------------------------------
// Displays the given text
// ----------------------------------------------------------------------
void T_DisplayTextScaled(int fontID, char* text, int x, int y, float scaleFactor)
{
    // The X we're currently drawing to
    int curDrawingX = x;

    // Current char in the text
    int curCIndex = 0;
    char curC = 0;

    // Width of the font
    int width = tomentdatapack.fontsheets[fontID].width;

    int curY = y; // y can be affected by \n

    // While we're not finished drawing
    while(text[curCIndex] != '\0')
    {
        if(text[curCIndex] == '\n')
        {
            curY += width * scaleFactor;
            curDrawingX = x;
            curCIndex++;
        }
        else
        {
            // Get the char we have to draw
            curC = text[curCIndex];

            // Translate to the sprite sheet coords to get the correct texture
            int texX, texY;
            T_TranslateASCIIToSpriteSheetCoords(curC, &texX, &texY);

            // Blit it on the screen
            SDL_Rect screenPos = {curDrawingX, curY, width * scaleFactor, width* scaleFactor};
            SDL_Rect size = {(texX * width), (texY * width), width, width};
            R_BlitIntoScreenScaled(&size, tomentdatapack.fontsheets[fontID].texture, &screenPos);

            // Next
            curCIndex++;
            curDrawingX += tomentdatapack.fontsheets[fontID].glyphsWidth[texY][texX] * scaleFactor;
        }
    }
}

// ----------------------------------------------------------------------
// Given a char, returns the sprite sheet coords, most naive approach ever
//
// This translation also kills the universality of the Text Renderer by forcing the font sheet to be 16x6
// ----------------------------------------------------------------------
void T_TranslateASCIIToSpriteSheetCoords(char c, int* destX, int* destY)
{
    switch(c)
    {
        case ' ':
            *destX = 0;
            *destY = 0;
            break;
        
        case '!':
            *destX = 1;
            *destY = 0;
            break;

        case '"':
            *destX = 2;
            *destY = 0;
            break;

        case '#':
            *destX = 3;
            *destY = 0;
            break;

        case '$':
            *destX = 4;
            *destY = 0;
            break;

        case '%':
            *destX = 5;
            *destY = 0;
            break;

        case '&':
            *destX = 6;
            *destY = 0;
            break;

        case '\'':
            *destX = 7;
            *destY = 0;
            break;

        case '(':
            *destX = 8;
            *destY = 0;
            break;

        case ')':
            *destX = 9;
            *destY = 0;
            break;

        case '*':
            *destX = 10;
            *destY = 0;
            break;

        case '+':
            *destX = 11;
            *destY = 0;
            break;

        case ',':
            *destX = 12;
            *destY = 0;
            break;

        case '-':
            *destX = 13;
            *destY = 0;
            break;

        case '.':
            *destX = 14;
            *destY = 0;
            break;

        case '/':
            *destX = 15;
            *destY = 0;
            break;

        case '0':
            *destX = 0;
            *destY = 1;
            break;
        
        case '1':
            *destX = 1;
            *destY = 1;
            break;

        case '2':
            *destX = 2;
            *destY = 1;
            break;

        case '3':
            *destX = 3;
            *destY = 1;
            break;

        case '4':
            *destX = 4;
            *destY = 1;
            break;

        case '5':
            *destX = 5;
            *destY = 1;
            break;

        case '6':
            *destX = 6;
            *destY = 1;
            break;

        case '7':
            *destX = 7;
            *destY = 1;
            break;

        case '8':
            *destX = 8;
            *destY = 1;
            break;

        case '9':
            *destX = 9;
            *destY = 1;
            break;

        case ':':
            *destX = 10;
            *destY = 1;
            break;

        case ';':
            *destX = 11;
            *destY = 1;
            break;

        case '<':
            *destX = 12;
            *destY = 1;
            break;

        case '=':
            *destX = 13;
            *destY = 1;
            break;

        case '>':
            *destX = 14;
            *destY = 1;
            break;

        case '?':
            *destX = 15;
            *destY = 1;
            break;

        case '@':
            *destX = 0;
            *destY = 2;
            break;
        
        case 'A':
            *destX = 1;
            *destY = 2;
            break;

        case 'B':
            *destX = 2;
            *destY = 2;
            break;

        case 'C':
            *destX = 3;
            *destY = 2;
            break;

        case 'D':
            *destX = 4;
            *destY = 2;
            break;

        case 'E':
            *destX = 5;
            *destY = 2;
            break;

        case 'F':
            *destX = 6;
            *destY = 2;
            break;

        case 'G':
            *destX = 7;
            *destY = 2;
            break;

        case 'H':
            *destX = 8;
            *destY = 2;
            break;

        case 'I':
            *destX = 9;
            *destY = 2;
            break;

        case 'J':
            *destX = 10;
            *destY = 2;
            break;

        case 'K':
            *destX = 11;
            *destY = 2;
            break;

        case 'L':
            *destX = 12;
            *destY = 2;
            break;

        case 'M':
            *destX = 13;
            *destY = 2;
            break;

        case 'N':
            *destX = 14;
            *destY = 2;
            break;

        case 'O':
            *destX = 15;
            *destY = 2;
            break;
        
        case 'P':
            *destX = 0;
            *destY = 3;
            break;
        
        case 'Q':
            *destX = 1;
            *destY = 3;
            break;

        case 'R':
            *destX = 2;
            *destY = 3;
            break;

        case 'S':
            *destX = 3;
            *destY = 3;
            break;

        case 'T':
            *destX = 4;
            *destY = 3;
            break;

        case 'U':
            *destX = 5;
            *destY = 3;
            break;

        case 'V':
            *destX = 6;
            *destY = 3;
            break;

        case 'W':
            *destX = 7;
            *destY = 3;
            break;

        case 'X':
            *destX = 8;
            *destY = 3;
            break;

        case 'Y':
            *destX = 9;
            *destY = 3;
            break;

        case 'Z':
            *destX = 10;
            *destY = 3;
            break;

        case '[':
            *destX = 11;
            *destY = 3;
            break;

        case '\\':
            *destX = 12;
            *destY = 3;
            break;

        case ']':
            *destX = 13;
            *destY = 3;
            break;

        case '^':
            *destX = 14;
            *destY = 3;
            break;

        case '_':
            *destX = 15;
            *destY = 3;
            break;

        case '`':
            *destX = 0;
            *destY = 4;
            break;
        
        case 'a':
            *destX = 1;
            *destY = 4;
            break;

        case 'b':
            *destX = 2;
            *destY = 4;
            break;

        case 'c':
            *destX = 3;
            *destY = 4;
            break;

        case 'd':
            *destX = 4;
            *destY = 4;
            break;

        case 'e':
            *destX = 5;
            *destY = 4;
            break;

        case 'f':
            *destX = 6;
            *destY = 4;
            break;

        case 'g':
            *destX = 7;
            *destY = 4;
            break;

        case 'h':
            *destX = 8;
            *destY = 4;
            break;

        case 'i':
            *destX = 9;
            *destY = 4;
            break;

        case 'j':
            *destX = 10;
            *destY = 4;
            break;

        case 'k':
            *destX = 11;
            *destY = 4;
            break;

        case 'l':
            *destX = 12;
            *destY = 4;
            break;

        case 'm':
            *destX = 13;
            *destY = 4;
            break;

        case 'n':
            *destX = 14;
            *destY = 4;
            break;

        case 'o':
            *destX = 15;
            *destY = 4;
            break;

        case 'p':
            *destX = 0;
            *destY = 5;
            break;
        
        case 'q':
            *destX = 1;
            *destY = 5;
            break;

        case 'r':
            *destX = 2;
            *destY = 5;
            break;

        case 's':
            *destX = 3;
            *destY = 5;
            break;

        case 't':
            *destX = 4;
            *destY = 5;
            break;

        case 'u':
            *destX = 5;
            *destY = 5;
            break;

        case 'v':
            *destX = 6;
            *destY = 5;
            break;

        case 'w':
            *destX = 7;
            *destY = 5;
            break;

        case 'x':
            *destX = 8;
            *destY = 5;
            break;

        case 'y':
            *destX = 9;
            *destY = 5;
            break;

        case 'z':
            *destX = 10;
            *destY = 5;
            break;

        case '{':
            *destX = 11;
            *destY = 5;
            break;

        case '|':
            *destX = 12;
            *destY = 5;
            break;

        case '}':
            *destX = 13;
            *destY = 5;
            break;

        case '~':
            *destX = 14;
            *destY = 5;
            break;

        case 127:
            *destX = 15;
            *destY = 5;
            break;

        default:
            *destX = 15;
            *destY = 5;
            break;
    }
}