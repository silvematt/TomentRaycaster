#include <stdio.h>
#include <string.h>
#include <math.h>

#include "R_Rendering.h"
#include "A_Application.h"
#include "G_Player.h"
#include "M_Map.h"
#include "I_InputHandling.h"
#include "D_AssetsManager.h"

uint32_t r_blankColor;
uint32_t r_transparencyColor;
uint32_t r_debugColor;


unsigned int* screenBuffers[5];
SDL_Rect dirtybox;
// ---------------------------------------------------------
//  SCREEN BUFFERS
// ---------------------------------------------------------

//-------------------------------------
// Initializes the rendering 
//-------------------------------------
void R_InitRendering(void)
{
    r_blankColor = SDL_MapRGB(win_surface->format, 0, 0, 0);
    r_transparencyColor = SDL_MapRGB(win_surface->format, 26, 24, 0);
    r_debugColor = SDL_MapRGB(win_surface->format, 0, 255, 0);

    // Clear initial render
    R_ClearRendering();

    // Allocate for screen buffers
    unsigned int* base = malloc((SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(unsigned int)) * 5);

    // Set screen buffers
    for(int i = 0; i < 5; i++)
    {
        screenBuffers[i] = (base + (SCREEN_WIDTH * SCREEN_HEIGHT* i));
    }
}


void R_RenderDev(void)
{
    R_DrawBackground();
    R_Raycast();
    R_DrawMinimap();
}

//-------------------------------------
// Fill buffers and put framebuffers on top of each other
//-------------------------------------
void R_ComposeFrame(void)
{
    switch(application.gamestate)
    {
        case dev:
            R_RenderDev();
        break;
    }
}

//-------------------------------------
// Gets a pixel from a surface
//-------------------------------------
Uint32 R_GetPixelFromSurface(SDL_Surface *surface, int x, int y)
{
    SDL_LockSurface(surface);
    Uint32* target_pixel = (Uint32 *) ((Uint8 *) surface->pixels
                        + ((int)(y)) * surface->pitch
                        + x * surface->format->BytesPerPixel);
    SDL_UnlockSurface(surface);

    return *target_pixel;
}

//-------------------------------------
// Reads the framebuffer with the dirtybox and transfer to win_surface
//-------------------------------------
void R_UpdateNoBlit(void)
{
    memcpy(pixels, screenBuffers[0], SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(unsigned int));
}

//-------------------------------------
// Updates the screen to the win_surface
//-------------------------------------
void R_FinishUpdate(void)
{
    SDL_UpdateWindowSurface(application.win);
}

//-------------------------------------
// Given an SDL_Surface, extracts the pixels of it and puts them in the selected framebuffer
//-------------------------------------
void R_BlitIntoBuffer(int buffer, SDL_Surface* sur, SDL_Rect* pos)
{
    unsigned int* spix = sur->pixels;
    Uint32 * target_pixel;

    for(int y = 0; y < pos->h; y++)
        for(int x = 0; x < pos->w; x++) 
        {
            // Extract pixel
            target_pixel = (Uint32 *) ((Uint8 *) sur->pixels
                            + y * sur->pitch
                            + x * sur->format->BytesPerPixel);

            if(*target_pixel == r_transparencyColor)
                continue; // Skip transparency

            // Put it into buffer 
            screenBuffers[buffer][(pos->x + x) + ((pos->y + y) * SCREEN_WIDTH)] = * target_pixel;
        }
}

//-------------------------------------
// Given an SDL_Surface, extracts the pixels of it and puts them in the selected framebuffer
//-------------------------------------
void R_BlitColorIntoBuffer(int buffer, int color, SDL_Rect* pos)
{
    for(int y = 0; y < pos->h; y++)
        for(int x = 0; x < pos->w; x++) 
        {
            // Put it into buffer
            screenBuffers[buffer][(pos->x + x) + ((pos->y + y) * SCREEN_WIDTH)] = color;
        }
}


//-------------------------------------
// Sets the screen to r_blankColor
//-------------------------------------
void R_ClearRendering(void)
{
    for(int y = 0; y < SCREEN_HEIGHT; y++)
        for(int x = 0; x < SCREEN_WIDTH; x++)
            pixels[x + (y * win_surface->w)] = r_blankColor;
}

//-------------------------------------
// Draw lines using Bresenham's 
//-------------------------------------
void R_DrawLineLow(int x0, int y0, int x1, int y1, int color)
{
    int dx = x1 - x0;
    int dy = y1 - y0;
    int yi = 1;
    if (dy < 0) 
    {
        yi = -1;
        dy = -dy;
    }

    int D = (2 * dy) - dx;
    int y = y0;

    for (int x = x0; x < x1; x++) 
    {
        if( x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT)    // To not go outside of boundaries
            screenBuffers[0][x + y * win_width] = color;
        if (D > 0)
        {
            y = y + yi;
            D = D + (2 * (dy - dx));
        }
        else
            D = D + 2*dy;
    }
}

//-------------------------------------
// Draw lines using Bresenham's 
//-------------------------------------
void R_DrawLineHigh(int x0, int y0, int x1, int y1, int color)
{
    int dx = x1 - x0;
    int dy = y1 - y0;
    int xi = 1;
    if (dx < 0)
    {
        xi = -1;
        dx = -dx;
    }
    int D = (2 * dx) - dy;
    int x = x0;

    for (int y = y0; y < y1; y++)
    {
        if( x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT)    // To not go outside of boundaries
            screenBuffers[0][x + y * win_width] = color;
        if (D > 0)
        {
            x = x + xi;
            D = D + (2 * (dx - dy));
        }
        else
            D = D + 2*dx;
    }
}

//-------------------------------------
// Draw lines using Bresenham's 
//-------------------------------------
void R_DrawLine(int x0, int y0, int x1, int y1, int color)
{
    if (abs(y1 - y0) < abs(x1 - x0)) 
    {
        if (x0 > x1)
                R_DrawLineLow(x1, y1, x0, y0, color);
            else
                R_DrawLineLow(x0, y0, x1, y1, color);
    }
        else
        {
            if (y0 > y1)
                R_DrawLineHigh(x1, y1, x0, y0, color);
            else
                R_DrawLineHigh(x0, y0, x1, y1, color);
        }
}

//------------------------------------------------
// Draws a single pixel on the current framebuffer
//-------------------------------------------------
void R_DrawPixel(int x, int y, int color)
{
    if( x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT)    // To not go outside of boundaries
            screenBuffers[0][x + y * win_width] = color;
}


//------------------------------------------------
// Draws a column of pixel on the current framebuffer
//-------------------------------------------------
void R_DrawColumn(int x, int y, int endY, int color)
{
    for(int i = y; i < endY; i++)
    {
        screenBuffers[0][x + i * win_width] = color;
    }
}

//-------------------------------------
// Draws the minimap
//-------------------------------------
void R_DrawMinimap(void)
{
    static bool mapInFramebuffer = false;
    SDL_Rect curRect;

    // If the map has never been drawn, draw it in the framebuffer 1
    if(!mapInFramebuffer)
    {
        for(int y = 0; y < MAP_HEIGHT; y++)
            for(int x = 0; x < MAP_WIDTH; x++)
            {
                // Set X and Y
                curRect.w = TILE_SIZE / MINIMAP_DIVIDER;
                curRect.h = TILE_SIZE / MINIMAP_DIVIDER;
                curRect.x = x * TILE_SIZE / MINIMAP_DIVIDER;
                curRect.y = y * TILE_SIZE / MINIMAP_DIVIDER;

                // If it is an empty space
                if(currentMap.map[y][x] == 0)
                {
                    R_BlitColorIntoBuffer(0, SDL_MapRGB(win_surface->format, 128, 128, 128), &curRect);
                }
                else
                {
                    R_BlitColorIntoBuffer(0, SDL_MapRGB(win_surface->format, 255, 0, 0), &curRect);
                }
            }
    }

    // Blit player in the framebuffer
    // Commented out because at this size it's too small
    /*
    player.surfaceRect.x = player.position.x / MINIMAP_DIVIDER;
    player.surfaceRect.y = player.position.y / MINIMAP_DIVIDER;
    player.surface->h = MINIMAP_PLAYER_HEIGHT;
    player.surface->w = MINIMAP_PLAYER_WIDTH;
    player.surfaceRect.h = MINIMAP_PLAYER_HEIGHT;
    player.surfaceRect.w = MINIMAP_PLAYER_WIDTH;
    R_BlitIntoBuffer(0, player.surface, &player.surfaceRect);
    */

    // Draw Direction
    R_DrawLine((player.centeredPos.x) / MINIMAP_DIVIDER, (player.centeredPos.y) / MINIMAP_DIVIDER, ((player.centeredPos.x)/MINIMAP_DIVIDER)+(playerinput.dir.x/MINIMAP_DIVIDER) * 25, ((player.centeredPos.y) / MINIMAP_DIVIDER)+(playerinput.dir.y / MINIMAP_DIVIDER) * 25, r_debugColor);
}

//-------------------------------------
// Draws the background of the game view
//-------------------------------------
void R_DrawBackground(void)
{
    // DRAW CEILING
    for(int x = 0; x < PROJECTION_PLANE_WIDTH; x++)
    {
        R_DrawColumn(x, 0, (PROJECTION_PLANE_HEIGHT-1) / 2, SDL_MapRGB(win_surface->format, 64, 64, 64));
    }

    // DRAW FLOOR
    for(int i = 0; i < PROJECTION_PLANE_WIDTH; i++)
    {
        R_DrawColumn(i, (PROJECTION_PLANE_HEIGHT-1) / 2, PROJECTION_PLANE_HEIGHT, SDL_MapRGB(win_surface->format, 128, 128, 128));
    }
}

//-------------------------------------
// Perfroms the Raycast and draws the walls
//-------------------------------------
void R_Raycast(void)
{
    //printf("ANGLE: %f\n", player.angle);
    //float rayAngle = player.angle;

    // The angle of the current ray
    float rayAngle = player.angle - (RADIAN * (PLAYER_FOV / 2));

    // Cast a ray foreach pixel of the projection plane
    for(int x = 0; x < PROJECTION_PLANE_WIDTH; x++)
    {
        // Fix angles
        if(rayAngle > 2*M_PI)
            rayAngle -= 2*M_PI;

        if(rayAngle < 0)
            rayAngle += 2*M_PI;

        // Hit distances
        float hDistance = 99999.9f;
        float vDistance = 99999.9f;

        // Steps the raycast took until hit
        int hDepth = 0;
        int vDepth = 0;

        vector2_t A;    // First intersection on the Horizontal Line
        vector2_t B;    // First intersection on the Vertical Line

        float Xa;       // Increment on the Horizontal check
        float Ya;       // Increment on the Vertical check
        
        // Current Horizontal Point
        float hcurx;
        float hcury;
        
        // Current Vertical point
        float vcurx;
        float vcury;

        int hobjectIDHit = -1;    // The ID of the object hit
        int vobjectIDHit = -1;    // The ID of the object hit
        int objectIDHit;
        
        vector2_t wallPoint;

        // HORIZONTAL CHECK
        if(rayAngle != 0.0f && rayAngle != M_PI) 
        {
            // Get first intersection
            if(rayAngle < M_PI)
            {
                //printf("FACING DOWN\n");
                A.y = floorf((player.centeredPos.y) / TILE_SIZE) * TILE_SIZE + TILE_SIZE;
                Ya = TILE_SIZE;
                Xa = TILE_SIZE / tan(rayAngle);
                A.x = (player.centeredPos.x) - ((player.centeredPos.y) - A.y) / tan(rayAngle);
            }
            else
            {
                //printf("FACING UP\n");
                A.y = floorf((player.centeredPos.y) / TILE_SIZE) * TILE_SIZE;
                Ya = -TILE_SIZE;
                Xa = TILE_SIZE / tan(rayAngle) *-1;
                A.x = (player.centeredPos.x) - ((player.centeredPos.y) - A.y) / tan(rayAngle);
            }

            // Set as current
            hcurx = A.x;
            hcury = A.y;

            // Check for wall, if not, check next grid
            while(hDepth < MAP_WIDTH)
            {
                // Current grid the ray is in
                int curGridX;
                int curGridY;
                
                if(rayAngle < M_PI) // Facing down
                {
                    curGridX = floor(hcurx / TILE_SIZE);
                    curGridY = floor(hcury / TILE_SIZE);
                }
                else
                {
                    // Here Y is reduced of 1 to make sure it goes to the upper grid instead of the collision point
                    curGridX = floor(hcurx / TILE_SIZE);
                    curGridY = floor((hcury-1) / TILE_SIZE);
                }

                // If the ray is in a grid that is inside the map
                if(curGridX >= 0 && curGridY >= 0 && curGridX < MAP_WIDTH && curGridY < MAP_HEIGHT)
                {
                    // If it hit a wall, register it, save the distance and get out of the while
                    if(currentMap.map[curGridY][curGridX] >= 1)
                    {
                        hDistance = fabs(sqrt((((player.centeredPos.x) - hcurx) * ((player.centeredPos.x) - hcurx)) + (((player.centeredPos.y) - hcury) * ((player.centeredPos.y) - hcury))));
                        hobjectIDHit = currentMap.map[curGridY][curGridX];
                        break;
                    }
                }
                else // If the ray went outisde the map the check fails, stop
                {
                    hDistance = 99999.9f;
                    break;
                }

                // If the ray hit an empty grid, check next
                hcurx += Xa;
                hcury += Ya;
                hDepth++;
            }

            //R_DrawLine(player.position.x, player.position.y, hcurx, hcury, r_debugColor);
        }
        else // The ray will never find an horizontal grid
            hDistance = 99999.9f;

        // VERICAL CHECK
        if(rayAngle != M_PI / 2 && rayAngle != (3*M_PI) / 2)
        {
            // Get first intersection
            if(rayAngle < M_PI / 2 || rayAngle > (3*M_PI) / 2)
            {
                //printf("FACING RIGHT\n");
                B.x = floorf((player.centeredPos.x)/ TILE_SIZE) * TILE_SIZE+TILE_SIZE;
                Ya = TILE_SIZE * tan(rayAngle);
                Xa = TILE_SIZE;
                B.y = (player.centeredPos.y) - ((player.centeredPos.x) - B.x) * tan(rayAngle);
            }
            else
            {
                //printf("FACING LEFT\n");
                B.x = floorf((player.centeredPos.x) / TILE_SIZE) * TILE_SIZE;
                Ya = TILE_SIZE * tan(rayAngle) *-1;
                Xa = -TILE_SIZE;
                B.y = (player.centeredPos.y) - ((player.centeredPos.x) - B.x) * tan(rayAngle);
            }

            // Set as current
            vcurx = B.x;
            vcury = B.y;

            // Check for wall, if not, check next grid
            while(vDepth < MAP_HEIGHT)
            {
                // Current grid the ray is in
                int curGridX;
                int curGridY;

                if(rayAngle < M_PI / 2 || rayAngle > (3*M_PI) / 2)
                {
                    curGridX = floor(vcurx / TILE_SIZE);
                    curGridY = floor(vcury / TILE_SIZE);
                }
                else
                {
                    // Here X is reduced of 1 to make sure it goes to the grid on the left of the collision point
                    curGridX = floor((vcurx-1) / TILE_SIZE);
                    curGridY = floor(vcury / TILE_SIZE);
                }
                
                // If the ray is in a grid that is inside the map
                if(curGridX >= 0 && curGridY >= 0 && curGridX < MAP_WIDTH && curGridY < MAP_HEIGHT)
                {
                    // If it hit a wall, register it, save the distance and get out of the while
                    if(currentMap.map[curGridY][curGridX] >= 1)
                        {
                            vDistance = fabs(sqrt((((player.centeredPos.x) - vcurx) * ((player.centeredPos.x) - vcurx)) + (((player.centeredPos.y) - vcury) * ((player.centeredPos.y) - vcury))));
                            vobjectIDHit = currentMap.map[curGridY][curGridX];
                            break;
                        }
                }
                else // If the ray went outisde the map the check fails, stop
                {
                    vDistance = 99999.9f;
                    break;
                }

                // If the ray hit an empty grid, check next
                vcurx += Xa;
                vcury += Ya;
                vDepth++;
            }

            //R_DrawLine(player.position.x, player.position.y, vcurx, vcury, SDL_MapRGB(win_surface->format, 255, 0,0));
        }
        else
            vDistance = 99999.9f;

        bool horizontal = false;    // Has this ray hit an horizontal?
        float correctDistance;      // Corrected distance for fixing fisheye

        // If the distances are the same we should skip, this happens very rarerly
        if(hDistance != vDistance)
        {
            // If the horizontal hit was closer
            if(hDistance < vDistance)
            {
                horizontal = true;
                correctDistance = hDistance;
                objectIDHit = hobjectIDHit;

                wallPoint.x = hcurx;
                wallPoint.y = hcury;

                if(DEBUG_RAYCAST_MINIMAP == 1)
                    R_DrawLine((player.centeredPos.x) / MINIMAP_DIVIDER, (player.centeredPos.y) / MINIMAP_DIVIDER, hcurx / MINIMAP_DIVIDER, hcury / MINIMAP_DIVIDER, r_debugColor);
            }
            else if (hDistance > vDistance)
            {
                horizontal = false;
                correctDistance = vDistance;
                objectIDHit = vobjectIDHit;

                wallPoint.x = vcurx;
                wallPoint.y = vcury;

                if(DEBUG_RAYCAST_MINIMAP == 1)
                    R_DrawLine((player.centeredPos.x) / MINIMAP_DIVIDER, (player.centeredPos.y) / MINIMAP_DIVIDER, vcurx / MINIMAP_DIVIDER, vcury / MINIMAP_DIVIDER, SDL_MapRGB(win_surface->format, 255, 255,0));
            }

        
            // Fix fisheye 
            float fixedAngle = rayAngle - player.angle;
            float finalDistance = correctDistance * cos(fixedAngle);

            // DRAW WALL
            float wallHeight = (TILE_SIZE  / finalDistance) * DISTANCE_TO_PROJECTION;
            float wallHeightUncapped = wallHeight;

            float wallOffset = (PROJECTION_PLANE_HEIGHT / 2) - (wallHeight / 2);    // Wall Y offset to draw them in the middle of the screen

            // Prevent from going off projection plane
            if(wallOffset < 0)
                wallOffset = 0;

            if(wallOffset > PROJECTION_PLANE_HEIGHT)
                wallOffset = PROJECTION_PLANE_HEIGHT;

            if(wallHeight > PROJECTION_PLANE_HEIGHT)
                wallHeight = PROJECTION_PLANE_HEIGHT;

            // Don't draw outside of the Projection Plane Height
            float end = wallOffset+wallHeight;
            end = SDL_clamp(end, 0, PROJECTION_PLANE_HEIGHT);

            object_t* curObject = tomentdatapack.objects[objectIDHit];
            // Check if object is null
            if(curObject->texture == NULL)
            {
                printf("WARNING! Trying to load an object with ID: %d, but it was never set in the objects\n", objectIDHit);
                rayAngle += (RADIAN * PLAYER_FOV) / PROJECTION_PLANE_WIDTH;
                return;
            }

            // Draw the walls as column of pixels
            if(horizontal) 
            {
                int offset = (int)hcurx % TILE_SIZE;

                // If looking down, flip the texture offset
                if(rayAngle < M_PI)
                    offset = (TILE_SIZE-1) - offset;
                    
                R_DrawColumnTextured( (x), wallOffset, end, curObject->texture, offset, wallHeightUncapped);
            }
            else
            {
                int offset = (int)vcury % TILE_SIZE;

                // If looking left, flip the texture offset
                if(rayAngle > M_PI / 2 && rayAngle < (3*M_PI) / 2)
                    offset = (TILE_SIZE-1) - offset;

                R_DrawColumnTextured( (x), wallOffset, end, (curObject->alt != NULL) ? curObject->alt->texture : curObject->texture, offset, wallHeightUncapped);
            }

            // Floor Casting & Ceiling
            float beta = (player.angle - rayAngle);
            if(beta > 2*M_PI)
                beta -= 2*M_PI;

            if(beta < 0)
                beta += 2*M_PI;

            for(int y = end+1; y < PROJECTION_PLANE_HEIGHT; y++)
            {
                // Get distance
                float straightlinedist = (32.0f * DISTANCE_TO_PROJECTION) / (y - PROJECTION_PLANE_CENTER);
                float d = straightlinedist / cos(beta);

                // Get coordinates
                float floorx = player.centeredPos.x + (cos(rayAngle) * d);
                float floory = player.centeredPos.y + (sin(rayAngle) * d);

                // Get textels
                int textureX = (int)floorx % 64;
                int textureY = (int)floory % 64;
                
                // Draw floor
                R_DrawPixel(x, y, R_GetPixelFromSurface(tomentdatapack.objects[W_Floor1]->texture, textureX, textureY));
                
                // Draw ceiling
                R_DrawPixel(x, PROJECTION_PLANE_HEIGHT-y, R_GetPixelFromSurface(tomentdatapack.objects[W_Ceiling1]->texture, textureX, textureY));
            }
        }
        
        // Check next ray
        rayAngle += (RADIAN * PLAYER_FOV) / PROJECTION_PLANE_WIDTH;
    }
}


//------------------------------------------------
// Draws a column of pixel on the current framebuffer
// x = column
// y = start of the wall (upper pixel)
// endY = last pixel of the wall
// texture = the texture to draw
// xOffset = the x index of the texture for this column
// wallheight = the height of the wall to be drawn (must be uncapped)
//-------------------------------------------------
void R_DrawColumnTextured(int x, int y, int endY, SDL_Surface* texture, int xOffset, float wallheight)
{
    Uint32* target_pixel;

    // The offset to extract Y pixels from the texture, this is != 0 only if the wall is bigger than the projection plane height
    float textureYoffset = 0.0f;

    // Correct the offset if the wall height is higher than the projection plane height
    if(wallheight > PROJECTION_PLANE_HEIGHT)
        textureYoffset = (wallheight-PROJECTION_PLANE_HEIGHT) / 2.0f;

    // Increment at each pixel
    float offset = TILE_SIZE / (wallheight);

    // The actual Y index
    float textureY = textureYoffset * offset;
    for(int i = y; i < endY; i++)
    {
        // Extract pixel
        target_pixel = (Uint32 *) ((Uint8 *) texture->pixels
                            + ((int)(textureY)) * texture->pitch
                            + xOffset * texture->format->BytesPerPixel);

        // Put it in the framebuffer
        screenBuffers[0][x + i * win_width] = *target_pixel;

        // Go forward
        textureY+= offset;
    }
}