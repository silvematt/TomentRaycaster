#include <stdio.h>
#include <string.h>
#include <math.h>

#include "R_Rendering.h"
#include "A_Application.h"
#include "G_Player.h"
#include "G_Game.h"
#include "I_InputHandling.h"
#include "D_AssetsManager.h"
#include "U_Utilities.h"

uint32_t r_blankColor;           // Color shown when nothing else is in the renderer
uint32_t r_transparencyColor;    // Color marked as "transparency", rendering of this color will be skipped for surfaces

uint32_t r_debugColor;

// ---------------------------------------------------------
//  SCREEN BUFFERS
// ---------------------------------------------------------
unsigned int* screenBuffers[5];
SDL_Rect dirtybox;

// Wall height
float wallHeights[PROJECTION_PLANE_WIDTH];

// Visible Sprite Determination
bool visibleTiles[MAP_HEIGHT][MAP_WIDTH];
sprite_t visibleSprites[MAXVISABLE];
int visibleSpritesLength;

// =========================================
// Thin wall Transparency
// =========================================
walldata_t currentThinWalls[PROJECTION_PLANE_WIDTH * MAX_THIN_WALL_TRANSPARENCY_RECURSION];
unsigned visibleThinWallsLength;

// Drawables
drawabledata_t allDrawables[MAX_DRAWABLES];
int allDrawablesLength;

//-------------------------------------
// Initializes the rendering 
//-------------------------------------
void R_InitRendering(void)
{
    r_blankColor = SDL_MapRGB(win_surface->format, 0, 0, 0);
    r_transparencyColor = SDL_MapRGB(win_surface->format, 155, 0, 155);
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

    // Initialize Doors //
    memset(doorstate, 0, (MAP_HEIGHT*MAP_WIDTH));
    
    // All doors start closed
    for(int y = 0; y < MAP_HEIGHT; y++)
        for(int x = 0; x < MAP_WIDTH; x++)
            doorpositions[y][x] = DOOR_FULLY_CLOSED;
}


//-------------------------------------
// Render routine
//-------------------------------------
void R_RenderDev(void)
{
    R_DrawBackground();
    R_Raycast();
    R_DrawDrawables();
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
                if(currentMap.wallMap[y][x] == 0)
                {
                    R_BlitColorIntoBuffer(0, SDL_MapRGB(win_surface->format, 128, 128, 128), &curRect);
                }
                else
                {
                    R_BlitColorIntoBuffer(0, SDL_MapRGB(win_surface->format, 255, 0, 0), &curRect);
                }

                if(DEBUG_VISIBLE_TILES_MINIMAP)
                {
                    if(visibleTiles[y][x])
                    {
                        R_BlitColorIntoBuffer(0, SDL_MapRGB(win_surface->format, 0, 255, 0), &curRect);
                    }
                }
            }

        if(DEBUG_VISIBLE_SPRITES_MINIMAP) 
        {
            for(int i = 0; i < visibleSpritesLength; i++)
            {
                // Set X and Y
                curRect.w = TILE_SIZE / MINIMAP_DIVIDER;
                curRect.h = TILE_SIZE / MINIMAP_DIVIDER;
                curRect.x = visibleSprites[i].gridPos.x * TILE_SIZE / MINIMAP_DIVIDER;
                curRect.y = visibleSprites[i].gridPos.y * TILE_SIZE / MINIMAP_DIVIDER;

                R_BlitColorIntoBuffer(0, SDL_MapRGB(win_surface->format, 0, 0, 255), &curRect);
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
    // Set the visibleTiles back to 0
    memset(visibleTiles, false, (MAP_HEIGHT*MAP_WIDTH));
    visibleSpritesLength = 0;
    visibleThinWallsLength = 0;
    allDrawablesLength = 0;
    
    // Thin wall recursion depth counter
    int thinWallDepth = 0;    

    //printf("ANGLE: %f\n", player.angle);
    //float rayAngle = player.angle;

    // The angle of the current ray
    float rayAngle = player.angle - (RADIAN * (PLAYER_FOV / 2));

    // Cast a ray foreach pixel of the projection plane
    for(int x = 0; x < PROJECTION_PLANE_WIDTH; x++)
    {
        thinWallDepth = 0;

        // Fix angles
        FIX_ANGLES(rayAngle);

        // Set thin wall tranparency array to 0 length

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
        
        // Current Horizontal grid Point
        int hcurGridX;
        int hcurGridY;

        // Current Vertical grid Point
        int vcurGridX;
        int vcurGridY;

        int hobjectIDHit = -1;    // The ID of the object hit
        int vobjectIDHit = -1;    // The ID of the object hit
        int objectIDHit;

        // Final grid point
        int fcurGridX;
        int fcurGridY;
        
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
                // Calculate grid point
                if(rayAngle < M_PI) // Facing down
                {
                    hcurGridX = floor(hcurx / TILE_SIZE);
                    hcurGridY = floor(hcury / TILE_SIZE);
                }
                else
                {
                    // Here Y is reduced of 1 to make sure it goes to the upper grid instead of the collision point
                    hcurGridX = floor(hcurx / TILE_SIZE);
                    hcurGridY = floor((hcury-1) / TILE_SIZE);
                }
                    
                // If the ray is in a grid that is inside the map
                if(hcurGridX >= 0 && hcurGridY >= 0 && hcurGridX < MAP_WIDTH && hcurGridY < MAP_HEIGHT)
                {
                    // Check for sprites
                    if(!visibleTiles[hcurGridY][hcurGridX] && currentMap.spritesMap[hcurGridY][hcurGridX] >= 1)
                    {
                        // Add sprite and data in the array

                        // Get Grid Pos
                        visibleSprites[visibleSpritesLength].gridPos.x = hcurGridX;
                        visibleSprites[visibleSpritesLength].gridPos.y = hcurGridY;

                        // Get World Pos
                        visibleSprites[visibleSpritesLength].pos.x = hcurGridX * TILE_SIZE;
                        visibleSprites[visibleSpritesLength].pos.y = hcurGridY * TILE_SIZE;

                        // Get Player Space pos
                        visibleSprites[visibleSpritesLength].pSpacePos.x = (visibleSprites[visibleSpritesLength].pos.x + (TILE_SIZE/2)) - player.centeredPos.x;
                        visibleSprites[visibleSpritesLength].pSpacePos.y = (visibleSprites[visibleSpritesLength].pos.y + (TILE_SIZE/2)) - player.centeredPos.y;

                        // Calculate the distance to player
                        visibleSprites[visibleSpritesLength].dist = sqrt(visibleSprites[visibleSpritesLength].pSpacePos.x*visibleSprites[visibleSpritesLength].pSpacePos.x + visibleSprites[visibleSpritesLength].pSpacePos.y*visibleSprites[visibleSpritesLength].pSpacePos.y);

                        // Get ID
                        visibleSprites[visibleSpritesLength].spriteID = currentMap.spritesMap[hcurGridY][hcurGridX];
                        
                        // Add it to the drawables
                        allDrawables[allDrawablesLength].type = DRWB_SPRITE;
                        allDrawables[allDrawablesLength].spritePtr = &visibleSprites[visibleSpritesLength];
                        
                        // Quick variable access
                        allDrawables[allDrawablesLength].dist = visibleSprites[visibleSpritesLength].dist;
                        
                        allDrawablesLength++;
                        visibleSpritesLength++;

                        // Mark this sprite as added so we don't get duplicates
                        visibleTiles[hcurGridY][hcurGridX] = true;
                    }
                    
                    int idHit = currentMap.wallMap[hcurGridY][hcurGridX];
                    // If it hit a wall, register it, save the distance and get out of the while
                    if(idHit >= 1)
                    {
                        // Check if this is a thin wall
                        if(U_GetBit(&tomentdatapack.walls[idHit]->flags, 0) == 1)
                        {
                            // Check if this is an Horzintal thin wall (if it is a vertical, just ignore it)
                            if(U_GetBit(&tomentdatapack.walls[idHit]->flags, 1) == 0)
                            {
                                // This is a thin wall, check if we hit it or if it was occluded by the wall

                                // Add half a tile to the current point
                                hcurx += (Xa/2);
                                hcury += (Ya/2);

                                // Calculate tile gird of the new point (added of a half a tile)
                                int newGridX = floor(hcurx / TILE_SIZE);
                                int newGridY = floor(hcury / TILE_SIZE);

                                // If the ray intersects with the door
                                if((hcurx - (64*newGridX)) < doorpositions[newGridY][newGridX])
                                {
                                    // If they're in the same tile, the door is visible
                                    if(newGridX == hcurGridX && newGridY == hcurGridY && thinWallDepth < MAX_THIN_WALL_TRANSPARENCY_RECURSION)
                                    {   
                                        hDistance = fabs(sqrt((((player.centeredPos.x) - hcurx) * ((player.centeredPos.x) - hcurx)) + (((player.centeredPos.y) - hcury) * ((player.centeredPos.y) - hcury))));;
                                        
                                        // Save the information about this hit, it will be drawn later after this ray draws a wall
                                        walldata_t* data = &currentThinWalls[visibleThinWallsLength];
                                        data->rayAngle = rayAngle;
                                        data->x = x;
                                        data->curX = hcurx;
                                        data->curY = hcury;
                                        data->distance = hDistance;
                                        data->gridPos.x = hcurGridX;
                                        data->gridPos.y = hcurGridY;
                                        data->idHit = currentMap.wallMap[hcurGridY][hcurGridX];
                                        data->isVertical = false;
                                        
                                        // Add it to the drawables
                                        allDrawables[allDrawablesLength].type = DRWB_WALL;
                                        allDrawables[allDrawablesLength].wallPtr = &currentThinWalls[visibleThinWallsLength];
                                        // Quick variable access
                                        allDrawables[allDrawablesLength].dist = data->distance;

                                        allDrawablesLength++;
                                        visibleThinWallsLength++;

                                        hcurx -= (Xa/2);
                                        hcury -= (Ya/2);

                                        thinWallDepth++;
                                        // break; don't break, keep casting the ray
                                    }
                                    else // otherwise it's not visible, revert the point and keep scanning
                                    {
                                        hcurx -= (Xa/2);
                                        hcury -= (Ya/2);
                                    }
                                }
                                else // otherwise the ray passes through the door (it's opening/closing)
                                {
                                    hcurx -= (Xa/2);
                                    hcury -= (Ya/2);
                                }
                            }
                        }
                        else // This is a normal wall
                        {
                            hDistance = fabs(sqrt((((player.centeredPos.x) - hcurx) * ((player.centeredPos.x) - hcurx)) + (((player.centeredPos.y) - hcury) * ((player.centeredPos.y) - hcury))));
                            hobjectIDHit = currentMap.wallMap[hcurGridY][hcurGridX];
                            break;
                        }
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
                // Calculate current grid the ray is in
                if(rayAngle < M_PI / 2 || rayAngle > (3*M_PI) / 2)
                {
                    vcurGridX = floor(vcurx / TILE_SIZE);
                    vcurGridY = floor(vcury / TILE_SIZE);
                }
                else
                {
                    // Here X is reduced of 1 to make sure it goes to the grid on the left of the collision point
                    vcurGridX = floor((vcurx-1) / TILE_SIZE);
                    vcurGridY = floor(vcury / TILE_SIZE);
                }
                
                // If the ray is in a grid that is inside the map
                if(vcurGridX >= 0 && vcurGridY >= 0 && vcurGridX < MAP_WIDTH && vcurGridY < MAP_HEIGHT)
                {
                    // Check for sprites
                    if(!visibleTiles[vcurGridY][vcurGridX] && currentMap.spritesMap[vcurGridY][vcurGridX] >= 1)
                    {
                        // Add sprite and data in the array

                        // Get Grid Pos
                        visibleSprites[visibleSpritesLength].gridPos.x = vcurGridX;
                        visibleSprites[visibleSpritesLength].gridPos.y = vcurGridY;

                        // Get World Pos
                        visibleSprites[visibleSpritesLength].pos.x = vcurGridX * TILE_SIZE;
                        visibleSprites[visibleSpritesLength].pos.y = vcurGridY * TILE_SIZE;

                        // Get Player Space pos
                        visibleSprites[visibleSpritesLength].pSpacePos.x = (visibleSprites[visibleSpritesLength].pos.x + (TILE_SIZE/2)) - player.centeredPos.x;
                        visibleSprites[visibleSpritesLength].pSpacePos.y = (visibleSprites[visibleSpritesLength].pos.y + (TILE_SIZE/2)) - player.centeredPos.y;

                        // Calculate the distance to player
                        visibleSprites[visibleSpritesLength].dist = sqrt((visibleSprites[visibleSpritesLength].pSpacePos.x*visibleSprites[visibleSpritesLength].pSpacePos.x) + (visibleSprites[visibleSpritesLength].pSpacePos.y*visibleSprites[visibleSpritesLength].pSpacePos.y));

                        // Get ID
                        visibleSprites[visibleSpritesLength].spriteID = currentMap.spritesMap[vcurGridY][vcurGridX];
                        
                        // Add it to the drawables
                        allDrawables[allDrawablesLength].type = DRWB_SPRITE;
                        allDrawables[allDrawablesLength].spritePtr = &visibleSprites[visibleSpritesLength];
                       
                        // Quick variable access
                        allDrawables[allDrawablesLength].dist = visibleSprites[visibleSpritesLength].dist;
                       
                        allDrawablesLength++;
                        
                        visibleSpritesLength++;

                        // Mark this sprite as added so we don't get duplicates
                        visibleTiles[vcurGridY][vcurGridX] = true;
                    }

                    int idHit = currentMap.wallMap[vcurGridY][vcurGridX];

                    // If it hit a wall, register it, save the distance and get out of the while
                    if(idHit >= 1)
                    {
                        // Check if this is a thin wall
                        if(U_GetBit(&tomentdatapack.walls[idHit]->flags, 0) == 1)
                        {
                             // Check if this is an Vertical thin wall (if it is an horizontal, just ignore it)
                            if(U_GetBit(&tomentdatapack.walls[idHit]->flags, 1) == 1)
                            {
                                // This is a thin wall, check if we hit it or if it was occluded by the wall

                                // Add half a tile to the current point
                                vcurx += (Xa/2);
                                vcury += (Ya/2);

                                // Calculate tile gird of the new point (added of a half a tile)
                                int newGridX = floor(vcurx / TILE_SIZE);
                                int newGridY = floor(vcury / TILE_SIZE);

                                // If the ray intersects with the door
                                if((vcury - (64*newGridY)) < doorpositions[newGridY][newGridX])
                                {
                                    // If they're in the same tile, the door is visible
                                    if(newGridX == vcurGridX && newGridY == vcurGridY)
                                    {
                                        vDistance = fabs(sqrt((((player.centeredPos.x) - vcurx) * ((player.centeredPos.x) - vcurx)) + (((player.centeredPos.y) - vcury) * ((player.centeredPos.y) - vcury))));;
                                        
                                        // Save the information about this hit, it will be drawn later after this ray draws a wall
                                        walldata_t* data = &currentThinWalls[visibleThinWallsLength];
                                        data->rayAngle = rayAngle;
                                        data->x = x;
                                        data->curX = vcurx;
                                        data->curY = vcury;
                                        data->distance = vDistance;
                                        data->gridPos.x = vcurGridX;
                                        data->gridPos.y = vcurGridY;
                                        data->idHit = currentMap.wallMap[vcurGridY][vcurGridX];
                                        data->isVertical = true;
                                        
                                        // Add it to the drawables
                                        allDrawables[allDrawablesLength].type = DRWB_WALL;
                                        allDrawables[allDrawablesLength].wallPtr = &currentThinWalls[visibleThinWallsLength];
                                        // Quick variable access
                                        allDrawables[allDrawablesLength].dist = data->distance;

                                        allDrawablesLength++;
                                        visibleThinWallsLength++;

                                        vcurx -= (Xa/2);
                                        vcury -= (Ya/2);

                                        thinWallDepth++;
                                        // break; don't break, keep casting the ray
                                    }
                                    else // otherwise it's not visible, revert the point and keep scanning
                                    {
                                        vcurx -= (Xa/2);
                                        vcury -= (Ya/2);
                                    }
                                }
                                else // otherwise the ray passes through the door (it's opening/closing)
                                {
                                    vcurx -= (Xa/2);
                                    vcury -= (Ya/2);
                                }
                            }
                        }
                        else
                        {
                            vDistance = fabs(sqrt((((player.centeredPos.x) - vcurx) * ((player.centeredPos.x) - vcurx)) + (((player.centeredPos.y) - vcury) * ((player.centeredPos.y) - vcury))));
                            vobjectIDHit = currentMap.wallMap[vcurGridY][vcurGridX];
                            break;
                        }
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

        
        // Get the in front grid of the player and save it (can be optimized)
        float firstHDistance = fabs(sqrt((((player.centeredPos.x) - A.x) * ((player.centeredPos.x) - A.x)) + (((player.centeredPos.y) - A.y) * ((player.centeredPos.y) - A.y))));
        float firstVDistance = fabs(sqrt((((player.centeredPos.x) - B.x) * ((player.centeredPos.x) - B.x)) + (((player.centeredPos.y) - B.y) * ((player.centeredPos.y) - B.y))));

        // Save the grid pos, 
        if(firstVDistance < firstHDistance)
        {
            if(rayAngle < M_PI / 2 || rayAngle > (3*M_PI) / 2)
            {
                player.inFrontGridPosition.x = floor(B.x / TILE_SIZE);
                player.inFrontGridPosition.y = floor(B.y / TILE_SIZE);
            }
            else
            {
                // Here X is reduced of 1 to make sure it goes to the grid on the left of the collision point
                player.inFrontGridPosition.x = floor((B.x-1) / TILE_SIZE);
                player.inFrontGridPosition.y = floor(B.y / TILE_SIZE);
            }
        }
        else
        {
            if(rayAngle < M_PI)
            {
                player.inFrontGridPosition.x = floor(A.x / TILE_SIZE);
                player.inFrontGridPosition.y = floor(A.y / TILE_SIZE);
            }
            else
            {
                // Here X is reduced of 1 to make sure it goes to the grid on the left of the collision point
                player.inFrontGridPosition.x = floor(A.x / TILE_SIZE);
                player.inFrontGridPosition.y = floor((A.y-1) / TILE_SIZE);
            }
        }

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

                fcurGridX = hcurGridX;
                fcurGridY = hcurGridY;

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

                fcurGridX = vcurGridX;
                fcurGridY = vcurGridY;

                if(DEBUG_RAYCAST_MINIMAP == 1)
                    R_DrawLine((player.centeredPos.x) / MINIMAP_DIVIDER, (player.centeredPos.y) / MINIMAP_DIVIDER, vcurx / MINIMAP_DIVIDER, vcury / MINIMAP_DIVIDER, r_debugColor);
            }

        
            // Fix fisheye 
            float fixedAngle = rayAngle - player.angle;
            float finalDistance = correctDistance * cos(fixedAngle);

            // DRAW WALL
            float wallHeight = (TILE_SIZE  / finalDistance) * DISTANCE_TO_PROJECTION;
            float wallHeightUncapped = wallHeight;

            // Save wall height
            wallHeights[x] = wallHeightUncapped;

            float wallOffset = (PROJECTION_PLANE_CENTER) - (wallHeight / 2);    // Wall Y offset to draw them in the middle of the screen

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

            // Calculate lighting intensity
            float wallLighting = (PLAYER_POINT_LIGHT_INTENSITY + currentMap.wallLight)  / finalDistance;
            wallLighting = SDL_clamp(wallLighting, 0, 1.0f);

            object_t* curObject = tomentdatapack.walls[objectIDHit];
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
                // Texture offset (shifted if it's a door)
                // Note:
                // For walls or closed doors the '- doorpos[]...' is always going to shift the curx 64.0 units
                // This is not a problem since it rounds back, shifting the offset by 64 puts us in the exact same place as if we never shifted, 
                // but this calculation lets us slide the texture offset too for the doors
                int offset = (int)(hcurx - (doorpositions[fcurGridY][fcurGridX])) % TILE_SIZE;

                // If looking down, flip the texture offset
                if(rayAngle < M_PI)
                    offset = (TILE_SIZE-1) - offset;
                    
                R_DrawColumnTexturedShaded((x), wallOffset+1, end+1, curObject->texture, offset, wallHeightUncapped, wallLighting);
            }
            else
            {
                // Texture offset (shifted if it's a door)
                // Note:
                // For walls or closed doors the '- doorpos[]...' is always going to shift the curx 64.0 units
                // This is not a problem since it rounds back, shifting the offset by 64 puts us in the exact same place as if we never shifted, 
                // but this calculation lets us slide the texture offse too for the doors
                int offset = (int)(vcury - (doorpositions[fcurGridY][fcurGridX])) % TILE_SIZE;

                // If looking left, flip the texture offset
                if(rayAngle > M_PI / 2 && rayAngle < (3*M_PI) / 2)
                    offset = (TILE_SIZE-1) - offset;

                R_DrawColumnTexturedShaded( (x), wallOffset+1, end+1, (curObject->alt != NULL) ? curObject->alt->texture : curObject->texture, offset, wallHeightUncapped, wallLighting);
            }

            R_FloorCastingAndCeiling(end, rayAngle, x);
        }

        // Check next ray
        rayAngle += (RADIAN * PLAYER_FOV) / PROJECTION_PLANE_WIDTH;
    }
}

//-------------------------------------
// Draw the passed thin wall
//-------------------------------------
void R_DrawThinWall(walldata_t* cur)
{
    // Fix fisheye 
    float fixedAngle = cur->rayAngle - player.angle;
    float finalDistance = cur->distance * cos(fixedAngle);

    // DRAW WALL
    float wallHeight = (TILE_SIZE  / finalDistance) * DISTANCE_TO_PROJECTION;
    float wallHeightUncapped = wallHeight;

    if(wallHeight < wallHeights[cur->x])
        return;

    float wallOffset = (PROJECTION_PLANE_CENTER) - (wallHeight / 2.0f);    // Wall Y offset to draw them in the middle of the screen

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

    // Calculate lighting intensity
    float wallLighting = (PLAYER_POINT_LIGHT_INTENSITY + currentMap.wallLight)  / finalDistance;
    wallLighting = SDL_clamp(wallLighting, 0, 1.0f);

    object_t* curObject = tomentdatapack.walls[cur->idHit];
    // Check if object is null
    if(curObject->texture == NULL)
    {
        printf("WARNING! Trying to load an object with ID: %d in thin wall loop, but it was never set in the objects\n", cur->idHit);
        return;
    }

    // Draw the walls as column of pixels
    if(!cur->isVertical) 
    {
        // Texture offset (shifted if it's a door)
        // Note:
        // For walls or closed doors the '- doorpos[]...' is always going to shift the curx 64.0 units
        // This is not a problem since it rounds back, shifting the offset by 64 puts us in the exact same place as if we never shifted, 
        // but this calculation lets us slide the texture offset too for the doors
        int offset = (int)(cur->curX - (doorpositions[cur->gridPos.y][cur->gridPos.x])) % TILE_SIZE;

        // If looking down, flip the texture offset
        if(cur->rayAngle < M_PI)
            offset = (TILE_SIZE-1) - offset;
            
        R_DrawColumnTexturedShaded((cur->x), wallOffset+1, end+1, curObject->texture, offset, wallHeightUncapped, wallLighting);
    }
    else
    {
        // Texture offset (shifted if it's a door)
        // Note:
        // For walls or closed doors the '- doorpos[]...' is always going to shift the curx 64.0 units
        // This is not a problem since it rounds back, shifting the offset by 64 puts us in the exact same place as if we never shifted, 
        // but this calculation lets us slide the texture offse too for the doors
        int offset = (int)(cur->curY - (doorpositions[cur->gridPos.y][cur->gridPos.x])) % TILE_SIZE;

        // If looking left, flip the texture offset
        if(cur->rayAngle > M_PI / 2 && cur->rayAngle < (3*M_PI) / 2)
            offset = (TILE_SIZE-1) - offset;

        R_DrawColumnTexturedShaded((cur->x), wallOffset+1, end+1, (curObject->alt != NULL) ? curObject->alt->texture : curObject->texture, offset, wallHeightUncapped, wallLighting);
    }
}

//-------------------------------------
// Floorcast and ceilingcast
// Params:
// - end = the end of the wall that states where to start to floorcast
// - rayAngle = the current rayangle
// - x = the x coordinate on the screen for this specific floor cast call
//-------------------------------------
void R_FloorCastingAndCeiling(float end, float rayAngle, int x)
{
    // Floor Casting & Ceiling
    float beta = (player.angle - rayAngle);
    FIX_ANGLES(beta);

    for(int y = floor(end); y < PROJECTION_PLANE_HEIGHT; y++)
    {
        // Get distance
        float straightlinedist = (RENDERING_PLAYER_HEIGHT * DISTANCE_TO_PROJECTION) / (y - PROJECTION_PLANE_CENTER);
        float d = straightlinedist / cos(beta);

        // Calculate lighting intensity
        float floorLighting = (PLAYER_POINT_LIGHT_INTENSITY + currentMap.floorLight) / d;
        floorLighting = SDL_clamp(floorLighting, 0, 1.0f);

        // Get coordinates
        float floorx = player.centeredPos.x + (cos(rayAngle) * d);
        float floory = player.centeredPos.y + (sin(rayAngle) * d);

        // Get textels
        int textureX = (int)floorx % TILE_SIZE;
        int textureY = (int)floory % TILE_SIZE;

        // Get map coordinates
        int curGridX = floor(floorx / TILE_SIZE);
        int curGridY = floor(floory / TILE_SIZE);

        int floorObjectID = -1;
        int ceilingObjectID = -1;

        // If the ray is in a grid that is inside the map
        if(curGridX >= 0 && curGridY >= 0 && curGridX < MAP_WIDTH && curGridY < MAP_HEIGHT)
        {
            // Check the floor texture at that point
            if(currentMap.floorMap[curGridY][curGridX] >= 1)
            {
                floorObjectID = currentMap.floorMap[curGridY][curGridX];
                
                // Draw floor
                R_DrawPixelShaded(x, y, R_GetPixelFromSurface(tomentdatapack.floors[floorObjectID]->texture, textureX, textureY), floorLighting);
            }

            // Check the ceiling texture at that point
            if(currentMap.ceilingMap[curGridY][curGridX] >= 1)
            {
                ceilingObjectID = currentMap.ceilingMap[curGridY][curGridX];

                // Draw ceiling
                R_DrawPixelShaded(x, PROJECTION_PLANE_HEIGHT-y, R_GetPixelFromSurface(tomentdatapack.ceilings[ceilingObjectID]->texture, textureX, textureY), floorLighting);
            }
        }
    }
}

//-------------------------------------
// Draws the passed sprite
//-------------------------------------
void R_DrawSprite(sprite_t* sprite)
{
    // Done in degrees to avoid computations (even if I could cache radians values and stuff)
    // Calculate angle and convert to degrees (*-1 makes sure it uses SDL screen space coordinates for unit circle and quadrants)
    float angle = ((atan2(-sprite->pSpacePos.y, sprite->pSpacePos.x))* RADIAN_TO_DEGREE)*-1;
    FIX_ANGLES_DEGREES(angle);

    float playerAngle = player.angle * RADIAN_TO_DEGREE;
    float yTemp = playerAngle + (PLAYER_FOV / 2) - angle;

    if(angle > 270 && playerAngle < 90)
        yTemp = playerAngle + (PLAYER_FOV / 2) - angle + 360;

    if(playerAngle > 270 && angle < 90)
        yTemp = playerAngle + (PLAYER_FOV / 2) - angle - 360;

    float spriteX = yTemp * (PROJECTION_PLANE_WIDTH / PLAYER_FOV_F);
    float spriteY = PROJECTION_PLANE_HEIGHT / 2;
    
    // Calculate distance and fix fisheye
    float fixedAngle = ((angle*RADIAN) - player.angle);
    float dist = (sprite->dist * cos(fixedAngle));

    sprite->height = DISTANCE_TO_PROJECTION * TILE_SIZE / dist;

    if(sprite->height <= 0)
        return;

    if(sprite->height > MAX_SPRITE_HEIGHT)
        sprite->height = MAX_SPRITE_HEIGHT;

    // Calculate lighting intensity
    float lighting = (PLAYER_POINT_LIGHT_INTENSITY + currentMap.floorLight) / dist;
    lighting = SDL_clamp(lighting, 0, 1.0f);

    // Draw
    int offset, drawX, drawYStart, drawYEnd;
    for(int j = 0; j < sprite->height; j++)
    {
        offset = j*TILE_SIZE/sprite->height;
        drawX = PROJECTION_PLANE_WIDTH-(spriteX)+j-(sprite->height/2);

        // Check clipping with wall
        if(wallHeights[drawX] >= sprite->height)
            continue;

        drawYStart = (SCREEN_HEIGHT / 2)-(sprite->height/2);
        drawYEnd = SCREEN_HEIGHT / 2 + sprite->height-(sprite->height/2);

        // Prevent overflow that causes sprites to lift up when too close
        if(drawYStart < 0) 
            drawYStart = 0;

        R_DrawColumnTexturedShaded(drawX, drawYStart, drawYEnd, tomentdatapack.sprites[sprite->spriteID]->texture,offset, sprite->height, lighting);
    }

    // Draws the center of the sprite
    //R_DrawPixel(PROJECTION_PLANE_WIDTH-spriteX, spriteY, SDL_MapRGB(win_surface->format, 255, 0, 0));
}

//-------------------------------------
// Drawables routine, sort and render drawables
//-------------------------------------
void R_DrawDrawables(void)
{
    U_QuicksortDrawables(allDrawables, 0, allDrawablesLength-1);

    for(int i = 0; i < allDrawablesLength; i++)
    {
        switch(allDrawables[i].type)
        {
            case DRWB_WALL:
                R_DrawThinWall(allDrawables[i].wallPtr);
                break;

            case DRWB_SPRITE:
                R_DrawSprite(allDrawables[i].spritePtr);
                break;
        }
    }
}

//-------------------------------------
// Gets a pixel from a surface
//-------------------------------------
Uint32 R_GetPixelFromSurface(SDL_Surface *surface, int x, int y)
{
    SDL_LockSurface(surface);
    Uint32* target_pixel = (Uint32 *) ((Uint8 *) surface->pixels
                        + y * surface->pitch
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
// Draws a single pixel on the current framebuffer
//-------------------------------------------------
void R_DrawPixelShaded(int x, int y, int color, float intensity)
{
    if( x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT)    // To not go outside of boundaries
    {
        // Do shading
        Uint8 r,g,b;
        SDL_GetRGB(color, win_surface->format, &r, &g, &b);
        r*=intensity;
        g*=intensity;
        b*=intensity;

        // Put it in the framebuffer
        screenBuffers[0][x + y * win_width] = SDL_MapRGB(win_surface->format, r,g,b);
    }
}


//------------------------------------------------
// Draws a column of pixel on the current framebuffer
//-------------------------------------------------
void R_DrawColumn(int x, int y, int endY, int color)
{
    for(int i = y; i < endY; i++)
    {
        if(x < PROJECTION_PLANE_WIDTH && x >= 0) // Don't overflow
            screenBuffers[0][x + i * win_width] = color;
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
        Uint32 pixel = R_GetPixelFromSurface(texture, xOffset, textureY);

        // Put it in the framebuffer
        if(x < PROJECTION_PLANE_WIDTH && x >= 0 && i < PROJECTION_PLANE_HEIGHT && i >= 0) // Don't overflow
            if(pixel != r_transparencyColor)
                screenBuffers[0][x + i * win_width] = pixel;

        // Go forward
        textureY+= offset;
    }
}

//------------------------------------------------
// Draws a column of pixel on the current framebuffer and applies shading
// x = column
// y = start of the wall (upper pixel)
// endY = last pixel of the wall
// texture = the texture to draw
// xOffset = the x index of the texture for this column
// wallheight = the height of the wall to be drawn (must be uncapped)
//-------------------------------------------------
void R_DrawColumnTexturedShaded(int x, int y, int endY, SDL_Surface* texture, int xOffset, float wallheight, float intensity)
{
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
        Uint32 pixel = R_GetPixelFromSurface(texture, xOffset, textureY);

        if(pixel != r_transparencyColor)
        {
            // Do shading
            Uint8 r,g,b;
            SDL_GetRGB(pixel, texture->format, &r, &g, &b);
            r*=intensity;
            g*=intensity;
            b*=intensity;

            // Put it in the framebuffer
            if(x < PROJECTION_PLANE_WIDTH && x >= 0 && i < PROJECTION_PLANE_HEIGHT && i >= 0) // Don't overflow
                screenBuffers[0][x + i * win_width] = SDL_MapRGB(texture->format, r,g,b);
        }

        // Go forward
        textureY+= offset;
    }
}