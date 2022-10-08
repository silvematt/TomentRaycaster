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
#include "T_TextRendering.h"
#include "G_MainMenu.h"
#include "G_Pathfinding.h"
#include "G_AI.h"
#include "G_Game.h"

uint32_t r_blankColor;           // Color shown when nothing else is in the renderer
uint32_t r_transparencyColor;    // Color marked as "transparency", rendering of this color will be skipped for surfaces
uint32_t r_debugColor;

// zBuffer
float zBuffer[PROJECTION_PLANE_HEIGHT][PROJECTION_PLANE_WIDTH];

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

bool debugRendering = false;
bool r_debugPathfinding = false;

// =========================================
// Static functions
// =========================================
// Save the information about this hit, it will be drawn later after this ray draws a wall
static void I_AddThinWall(int level, bool horizontal, float rayAngle, int x, float curX, float curY, int gridX, int gridY, float distance);
static void I_DebugPathfinding(void);

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
}


//-------------------------------------
// Render routine
//-------------------------------------
void R_RenderDev(void)
{
    // Render World
    R_DrawBackground();
    R_Raycast();
    
    // Render Player
    G_PlayerRender();

    // Render UI
    R_DrawMinimap();
    G_PlayerUIRender();

    T_DisplayTextScaled(FONT_BLKCRY, "This is an alert  message!", 360, 10, 1.0f);
}

//-------------------------------------
// Fill buffers and put framebuffers on top of each other
//-------------------------------------
void R_ComposeFrame(void)
{
    switch(application.gamestate)
    {
        case GSTATE_MENU:
            G_RenderCurrentMenuBackground();
            G_RenderCurrentMenu();
        break;

        case GSTATE_GAME:
            R_RenderDev();
        break;
    }
}

//-------------------------------------
// Draws the minimap
//-------------------------------------
void R_DrawMinimap(void)
{
    if(r_debugPathfinding)
    {
        I_DebugPathfinding();
        return;
    }

    SDL_Rect curRect;

    // If the map has never been drawn, draw it in the framebuffer 1
    for(int y = 0; y < MAP_HEIGHT; y++)
        for(int x = 0; x < MAP_WIDTH; x++)
        {
            // Set X and Y
            curRect.w = TILE_SIZE / MINIMAP_DIVIDER;
            curRect.h = TILE_SIZE / MINIMAP_DIVIDER;
            curRect.x = x * TILE_SIZE / MINIMAP_DIVIDER;
            curRect.y = y * TILE_SIZE / MINIMAP_DIVIDER;

            // If it is an empty space
            if(R_GetValueFromLevel(player.level, y, x) == 0)
            {
                R_BlitColorIntoScreen(SDL_MapRGB(win_surface->format, 128, 128, 128), &curRect);
            }
            else
            {
                R_BlitColorIntoScreen(SDL_MapRGB(win_surface->format, 255, 0, 0), &curRect);
            }

            if(DEBUG_VISIBLE_TILES_MINIMAP)
            {
                if(visibleTiles[y][x])
                {
                    R_BlitColorIntoScreen(SDL_MapRGB(win_surface->format, 0, 255, 0), &curRect);
                }
            }

            if(DEBUG_DYNAMIC_SPRITES)
            {
                if(G_GetFromDynamicSpriteMap(player.level, y, x) != NULL && G_GetFromDynamicSpriteMap(player.level, y, x)->base.active)
                {
                    R_BlitColorIntoScreen(SDL_MapRGB(win_surface->format, 0, 255, 0), &curRect);
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

            R_BlitColorIntoScreen(SDL_MapRGB(win_surface->format, 0, 0, 255), &curRect);
        }
    }

    // Debug Attack Cone:
    for(int i = 0; i < ATTACK_CONE_SIZE; i++)
    {
        // Set X and Y
        curRect.w = TILE_SIZE / MINIMAP_DIVIDER;
        curRect.h = TILE_SIZE / MINIMAP_DIVIDER;
        curRect.x = player.attackConeGridPos[i].x * TILE_SIZE / MINIMAP_DIVIDER;
        curRect.y = player.attackConeGridPos[i].y * TILE_SIZE / MINIMAP_DIVIDER;

        R_BlitColorIntoScreen(SDL_MapRGB(win_surface->format, 255, 0, 255), &curRect);
    }
    

    // Draw Direction
    R_DrawLine((player.centeredPos.x) / MINIMAP_DIVIDER, (player.centeredPos.y) / MINIMAP_DIVIDER, ((player.centeredPos.x)/MINIMAP_DIVIDER)+(playerinput.dir.x/MINIMAP_DIVIDER) * 25, ((player.centeredPos.y) / MINIMAP_DIVIDER)+(playerinput.dir.y / MINIMAP_DIVIDER) * 25, r_debugColor);
}

int scrollOffset = 0;
//-------------------------------------
// Draws the background of the game view
//-------------------------------------
void R_DrawBackground(void)
{
    if(currentMap.skyID == 0)
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
    else
    {
        // Blit the sky texture
        object_t* curSky = tomentdatapack.skies[currentMap.skyID];

        static SDL_Rect size = {PROJECTION_PLANE_WIDTH, 0, PROJECTION_PLANE_WIDTH, PROJECTION_PLANE_HEIGHT};
        static SDL_Rect pos =  {0, 0, PROJECTION_PLANE_WIDTH, PROJECTION_PLANE_HEIGHT};

        // Correct the offset (player.Angle * 611 is the sweet spot to make the shift back seamless when the angles are fixed (round(6.28318.. * 611) % 640 = 0) )
        scrollOffset = (int)round(player.angle*611) % PROJECTION_PLANE_WIDTH;
        
        // Limit the offset to max 640
        if(scrollOffset < -PROJECTION_PLANE_WIDTH || scrollOffset > PROJECTION_PLANE_WIDTH)
            scrollOffset = 0;

        // Shift the sky with the rotation
        size.x = scrollOffset;

        SDL_BlitScaled(curSky->texture, &size, win_surface, &pos);
    }
}

void I_Ray(int level, int playersLevel)
{
    // Initialize stuff for raycast
    memset(visibleTiles, false, MAP_HEIGHT*MAP_WIDTH*sizeof(bool));

    visibleSpritesLength = 0;
    visibleThinWallsLength = 0;
    allDrawablesLength = 0;

    float rayAngle = player.angle - (RADIAN * (PLAYER_FOV / 2));
    // Cast a ray foreach pixel of the projection plane
    for(int x = 0; x < PROJECTION_PLANE_WIDTH; x++)
    {
        bool occlusionEnabled = (level == playersLevel) ? true : false; // for now player can only be on level 0

        // Variables set by Raycast functions to draw floor
        float outHeight;
        float outEnd;

        if(occlusionEnabled)
            R_RaycastPlayersLevel(level, x, rayAngle);
        else
            R_RaycastLevelNoOcclusion(level, x, rayAngle);


        // Check next ray
        rayAngle += (RADIAN * PLAYER_FOV) / PROJECTION_PLANE_WIDTH;
    }

    // Perform post-raycast operations
    R_DrawDrawables();
}

//-------------------------------------
// Perfroms the Raycast routine
//-------------------------------------
void R_Raycast(void)
{
    // Reset wall heights (
    for(int y = 0; y < PROJECTION_PLANE_HEIGHT; y++)
        for(int x = 0; x < PROJECTION_PLANE_WIDTH; x++)
            zBuffer[y][x] = FLT_MAX;


    // Determine player's level
    player.level = (int)floor(player.z / TILE_SIZE);
    player.level = SDL_clamp(player.level, 0, MAX_N_LEVELS-1);

    for(int i = MAX_N_LEVELS-1; i >= player.level+1; i--)
    {
        I_Ray(i, player.level);
    }

    // Raycast all that's below
    for(int i = 0; i <= player.level-1; i++)
    {
        I_Ray(i, player.level);
    }

    // Raycast player's level
    I_Ray(player.level, player.level);
}

void R_RaycastPlayersLevel(int level, int x, float _rayAngle)
{
    // Fix angles
    float rayAngle = _rayAngle;
    FIX_ANGLES(rayAngle);

    // Set thin wall tranparency array to 0 length
    int thinWallDepth = 0;

    // Hit distances
    float hDistance = FLT_MAX;
    float vDistance = FLT_MAX;

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

        
        // Check for sprites in the player's cell
        // If the current tile wasn't already marked as a visible sprite && there is a sprite in that tile
        int spriteID = R_GetValueFromSpritesMap(player.level, player.gridPosition.y, player.gridPosition.x); 
        if(!visibleTiles[player.gridPosition.y][player.gridPosition.x])
        {   
            // Add sprite and data in the array
            if(spriteID >= 1)
                R_AddToVisibleSprite(player.gridPosition.x, player.gridPosition.y, player.level, spriteID);

            // Check if there's an active dynamic sprite there
            if(G_GetFromDynamicSpriteMap(level, player.gridPosition.y, player.gridPosition.x) != NULL && G_GetFromDynamicSpriteMap(level, player.gridPosition.y, player.gridPosition.x)->base.active)
                R_AddDynamicToVisibleSprite(level, player.gridPosition.x, player.gridPosition.y);

            // Check if there's an active dynamic sprite there
            if(G_GetFromDeadDynamicSpriteMap(level, player.gridPosition.y, player.gridPosition.x) != NULL && G_GetFromDeadDynamicSpriteMap(level, player.gridPosition.y, player.gridPosition.x)->base.active)
                R_AddDeadDynamicToVisibleSprite(level, player.gridPosition.x, player.gridPosition.y);
        }

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
                // If the current tile wasn't already marked as a visible sprite && there is a sprite in that tile
                int spriteID = R_GetValueFromSpritesMap(level, hcurGridY, hcurGridX); 
                if(!visibleTiles[hcurGridY][hcurGridX])
                {   
                    // Add sprite and data in the array
                    if(spriteID >= 1)
                        R_AddToVisibleSprite(hcurGridX, hcurGridY, level, spriteID);

                    // Check if there's an active dynamic sprite there
                    if(G_GetFromDynamicSpriteMap(level, hcurGridY, hcurGridX) != NULL && G_GetFromDynamicSpriteMap(level, hcurGridY, hcurGridX)->base.active)
                        R_AddDynamicToVisibleSprite(level, hcurGridX, hcurGridY);

                    // Check if there's an active dead dynamic sprite there
                    if(G_GetFromDeadDynamicSpriteMap(level, hcurGridY, hcurGridX) != NULL && G_GetFromDeadDynamicSpriteMap(level, hcurGridY, hcurGridX)->base.active)
                        R_AddDeadDynamicToVisibleSprite(level, hcurGridX, hcurGridY);
                }
                
                int idHit = R_GetValueFromLevel(level, hcurGridY, hcurGridX);
                // If it hit a wall, register it, save the distance and get out of the while
                if(idHit >= 1)
                {
                    // Check if this is a thin wall
                    if(U_GetBit(&tomentdatapack.walls[idHit]->flags, 0) == 1)
                    {
                        // Check if this is an Horzintal thin wall (if it is a vertical, just ignore it)
                        if(U_GetBit(&tomentdatapack.walls[idHit]->flags, 1) == 0)
                        {
                            // Add half a tile to the current point
                            hcurx += (Xa/2);
                            hcury += (Ya/2);

                            // Calculate tile gird of the new point (added of a half a tile)
                            int newGridX = floor(hcurx / TILE_SIZE);
                            int newGridY = floor(hcury / TILE_SIZE);

                            // If the ray intersects with the door
                                // If they're in the same tile, the door is visible
                                if(newGridX == hcurGridX && newGridY == hcurGridY && thinWallDepth < MAX_THIN_WALL_TRANSPARENCY_RECURSION)
                                {   
                                    hDistance = fabs(sqrt((((player.centeredPos.x) - hcurx) * ((player.centeredPos.x) - hcurx)) + (((player.centeredPos.y) - hcury) * ((player.centeredPos.y) - hcury))));
                                    
                                    // Save the information about this hit, it will be drawn later after this ray draws a wall
                                    I_AddThinWall(level, true, rayAngle, x, hcurx, hcury, hcurGridX, hcurGridY, hDistance);

                                    hcurx -= (Xa/2);
                                    hcury -= (Ya/2);

                                    thinWallDepth++;
                                    // break; don't break, keep casting the ray
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
                        hobjectIDHit = R_GetValueFromLevel(level, hcurGridY, hcurGridX);
                        break;
                    }
                }
            }
            else // If the ray went outisde the map the check fails, stop
            {
                hDistance = FLT_MAX;
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
        hDistance = FLT_MAX;

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
                int spriteID = R_GetValueFromSpritesMap(level, vcurGridY, vcurGridX);
                // Check for sprites
                if(!visibleTiles[vcurGridY][vcurGridX])
                {
                    // Add sprite and data in the array
                    if(spriteID >= 1)
                        R_AddToVisibleSprite(vcurGridX, vcurGridY, level, spriteID);

                    // Check if there's an active dynamic sprite there
                    if(G_GetFromDynamicSpriteMap(level, vcurGridY, vcurGridX) != NULL && G_GetFromDynamicSpriteMap(level, vcurGridY, vcurGridX)->base.active)
                        R_AddDynamicToVisibleSprite(level, vcurGridX, vcurGridY);

                    // Check if there's an active dynamic sprite there
                    if(G_GetFromDeadDynamicSpriteMap(level, vcurGridY, vcurGridX) != NULL && G_GetFromDeadDynamicSpriteMap(level, vcurGridY, vcurGridX)->base.active)
                        R_AddDeadDynamicToVisibleSprite(level, vcurGridX, vcurGridY);
                }

                int idHit = R_GetValueFromLevel(level, vcurGridY, vcurGridX);

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
                                // If they're in the same tile, the door is visible
                                if(newGridX == vcurGridX && newGridY == vcurGridY  && thinWallDepth < MAX_THIN_WALL_TRANSPARENCY_RECURSION)
                                {
                                    vDistance = fabs(sqrt((((player.centeredPos.x) - vcurx) * ((player.centeredPos.x) - vcurx)) + (((player.centeredPos.y) - vcury) * ((player.centeredPos.y) - vcury))));
                                    
                                    // Save the information about this hit, it will be drawn later after this ray draws a wall
                                    I_AddThinWall(level, false, rayAngle, x, vcurx, vcury, vcurGridX, vcurGridY, vDistance);

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
                    }
                    else
                    {
                        vDistance = fabs(sqrt((((player.centeredPos.x) - vcurx) * ((player.centeredPos.x) - vcurx)) + (((player.centeredPos.y) - vcury) * ((player.centeredPos.y) - vcury))));
                        vobjectIDHit = R_GetValueFromLevel(level, vcurGridY, vcurGridX);
                        break;
                    }
                }
            }
            else // If the ray went outisde the map the check fails, stop
            {
                vDistance = FLT_MAX;
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
        vDistance = FLT_MAX;

    
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

        float screenZ = floor(DISTANCE_TO_PROJECTION / finalDistance*(player.z-(TILE_SIZE/2)));
        int wallOffset = (PROJECTION_PLANE_CENTER) - floor(wallHeight / 2.0f) + screenZ;    // Wall Y offset to draw them in the middle of the screen + z
        
        //int wallOffset =  (PROJECTION_PLANE_HEIGHT-wallHeight)/2 + screenZ;
        
        int end = floor(wallOffset+wallHeight);
        int start = floor(wallOffset);

        float leveledStart = start - floor(wallHeight)*level;
        float leveledEnd = end - floor(wallHeight)*level+1;

        // Check if start and end are offscreen, if so, don't draw the walls, but draw the bottom/top regardless
        bool isOffScreenBottom = (leveledStart > PROJECTION_PLANE_HEIGHT);
        if(leveledEnd > PROJECTION_PLANE_HEIGHT)
            leveledEnd = PROJECTION_PLANE_HEIGHT;

        // Calculate lighting intensity
        float wallLighting = (PLAYER_POINT_LIGHT_INTENSITY + currentMap.wallLight)  / finalDistance;
        wallLighting = SDL_clamp(wallLighting, 0, 1.0f);

        object_t* curObject = tomentdatapack.walls[objectIDHit];

        // Draw the walls as column of pixels
        if(horizontal && !isOffScreenBottom) 
        {
            // Texture offset (shifted if it's a door)
            // Note:
            // For walls or closed doors the '- doorpos[]...' is always going to shift the curx 64.0 units
            // This is not a problem since it rounds back, shifting the offset by 64 puts us in the exact same place as if we never shifted, 
            // but this calculation lets us slide the texture offset too for the doors
            int offset = (int)(hcurx - (G_GetDoorPosition(level, fcurGridY, fcurGridX))) % TILE_SIZE;

            // If looking down, flip the texture offset
            if(rayAngle < M_PI)
                offset = (TILE_SIZE-1) - offset;
            
            R_DrawStripeTexturedShaded((x), leveledStart, leveledEnd, curObject->texture, offset, wallHeightUncapped, wallLighting, finalDistance);

            if(debugRendering)
            {
                SDL_Delay(10);
                SDL_UpdateWindowSurface(application.win);
            }
        }
        else if(!isOffScreenBottom)
        {
            // Texture offset (shifted if it's a door)
            // Note:
            // For walls or closed doors the '- doorpos[]...' is always going to shift the curx 64.0 units
            // This is not a problem since it rounds back, shifting the offset by 64 puts us in the exact same place as if we never shifted, 
            // but this calculation lets us slide the texture offse too for the doors
            int offset = (int)(vcury - (G_GetDoorPosition(level, fcurGridY, fcurGridX))) % TILE_SIZE;
            offset = SDL_clamp(offset, 0, TILE_SIZE);

            // If looking left, flip the texture offset
            if(rayAngle > M_PI / 2 && rayAngle < (3*M_PI) / 2)
                offset = (TILE_SIZE-1) - offset;

            R_DrawStripeTexturedShaded((x), leveledStart, leveledEnd, (curObject->alt != NULL) ? curObject->alt->texture : curObject->texture, offset, wallHeightUncapped, wallLighting, finalDistance);

            if(debugRendering)
            {
                SDL_Delay(10);
                SDL_UpdateWindowSurface(application.win);
            }
        }

        if(level == 0)
            R_FloorCasting(end, rayAngle, x, wallHeight);
        
        if(currentMap.hasAbsCeiling)
            R_CeilingCasting(currentMap.absCeilingLevel, start, rayAngle, x, wallHeight);

        //R_DrawPixel(x, start, r_debugColor);
    }
}


void R_RaycastLevelNoOcclusion(int level, int x, float _rayAngle)
{
    // Fix angles
    float rayAngle = _rayAngle;
    FIX_ANGLES(rayAngle);

    // Set thin wall tranparency array to 0 length
    int thinWallDepth = 0;

    // Hit distances
    float hDistance = FLT_MAX;
    float vDistance = FLT_MAX;

    // Steps the raycast took until hit
    int hDepth = 0;
    int vDepth = 0;

    vector2_t A;    // First intersection on the Horizontal Line
    vector2_t B;    // First intersection on the Vertical Line

    float horXa;       // Increment on the Horizontal check
    float horYa;       // Increment on the Vertical check
    
    float verXa;       // Increment on the Horizontal check
    float verYa;       // Increment on the Vertical check

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

    // For NonOccluded raycast we should keep raycasting until the end or the map or the defined 
    int maxDrawDistance = 24;
    bool mapEndedOrMaxDistanceReached = false;
    walldata_t toDraw[PROJECTION_PLANE_WIDTH * MAX_N_LEVELS];
    unsigned int toDrawLength = 0;
    
    // HORIZONTAL CHECK
    if(rayAngle != 0.0f && rayAngle != M_PI) 
    {
        // Get first intersection
        if(rayAngle < M_PI)
        {
            //printf("FACING DOWN\n");
            A.y = floorf((player.centeredPos.y) / TILE_SIZE) * TILE_SIZE + TILE_SIZE;
            horYa = TILE_SIZE;
            horXa = TILE_SIZE / tan(rayAngle);
            A.x = (player.centeredPos.x) - ((player.centeredPos.y) - A.y) / tan(rayAngle);
        }
        else
        {
            //printf("FACING UP\n");
            A.y = floorf((player.centeredPos.y) / TILE_SIZE) * TILE_SIZE;
            horYa = -TILE_SIZE;
            horXa = TILE_SIZE / tan(rayAngle) *-1;
            A.x = (player.centeredPos.x) - ((player.centeredPos.y) - A.y) / tan(rayAngle);
        }

        // Set as current
        hcurx = A.x;
        hcury = A.y;
    }
    else // The ray will never find an horizontal grid
        hDistance = FLT_MAX;

    // VERICAL CHECK
    if(rayAngle != M_PI / 2 && rayAngle != (3*M_PI) / 2)
    {
        // Get first intersection
        if(rayAngle < M_PI / 2 || rayAngle > (3*M_PI) / 2)
        {
            //printf("FACING RIGHT\n");
            B.x = floorf((player.centeredPos.x)/ TILE_SIZE) * TILE_SIZE+TILE_SIZE;
            verYa = TILE_SIZE * tan(rayAngle);
            verXa = TILE_SIZE;
            B.y = (player.centeredPos.y) - ((player.centeredPos.x) - B.x) * tan(rayAngle);
        }
        else
        {
            //printf("FACING LEFT\n");
            B.x = floorf((player.centeredPos.x) / TILE_SIZE) * TILE_SIZE;
            verYa = TILE_SIZE * tan(rayAngle) *-1;
            verXa = -TILE_SIZE;
            B.y = (player.centeredPos.y) - ((player.centeredPos.x) - B.x) * tan(rayAngle);
        }

        // Set as current
        vcurx = B.x;
        vcury = B.y;
    }
    else // The ray will never find an vertical grid
        vDistance = FLT_MAX;

    int depth = 0;
    bool horEnded = false;
    bool verEnded = false;
    while(depth < MAP_WIDTH && mapEndedOrMaxDistanceReached == false)
    {
        bool thinWallHit = false;

        bool horHitted = false, verHitted = false;
        hDistance = FLT_MAX;
        vDistance = FLT_MAX;

        // ---------------------
        // HORIZONTAL
        // ----------------------
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
            // If the current tile wasn't already marked as a visible sprite && there is a sprite in that tile
            int spriteID = R_GetValueFromSpritesMap(level, hcurGridY, hcurGridX);
            if(!visibleTiles[hcurGridY][hcurGridX])
            {
                // Add sprite and data in the array
                if(spriteID >= 1)
                    R_AddToVisibleSprite(hcurGridX, hcurGridY, level, spriteID);

                // Check if there's an active dynamic sprite there
                if(G_GetFromDynamicSpriteMap(level, hcurGridY, hcurGridX) != NULL && G_GetFromDynamicSpriteMap(level, hcurGridY, hcurGridX)->base.active)
                    R_AddDynamicToVisibleSprite(level, hcurGridX, hcurGridY);

                // Check if there's an active dynamic sprite there
                if(G_GetFromDeadDynamicSpriteMap(level, hcurGridY, hcurGridX) != NULL && G_GetFromDeadDynamicSpriteMap(level, hcurGridY, hcurGridX)->base.active)
                    R_AddDeadDynamicToVisibleSprite(level, hcurGridX, hcurGridY);
            }
            
            int idHit = R_GetValueFromLevel(level, hcurGridY, hcurGridX);
            // If it hit a wall, register it, save the distance and get out of the while
            if(idHit >= 1)
            {
                // Check if this is a thin wall
                if(U_GetBit(&tomentdatapack.walls[idHit]->flags, 0) == 1)
                {
                        thinWallHit = true;

                    // Check if this is an Horzintal thin wall (if it is a vertical, just ignore it)
                    if(U_GetBit(&tomentdatapack.walls[idHit]->flags, 1) == 0)
                    {
                        // Add half a tile to the current point
                        hcurx += (horXa/2);
                        hcury += (horYa/2);

                        // Calculate tile gird of the new point (added of a half a tile)
                        int newGridX = floor(hcurx / TILE_SIZE);
                        int newGridY = floor(hcury / TILE_SIZE);

                        // If the ray intersects with the door
                            // If they're in the same tile, the door is visible
                            if(newGridX == hcurGridX && newGridY == hcurGridY && thinWallDepth < MAX_THIN_WALL_TRANSPARENCY_RECURSION)
                            {   
                                hDistance = fabs(sqrt((((player.centeredPos.x) - hcurx) * ((player.centeredPos.x) - hcurx)) + (((player.centeredPos.y) - hcury) * ((player.centeredPos.y) - hcury))));
                                
                                // Save the information about this hit, it will be drawn later after this ray draws a wall
                                I_AddThinWall(level, true, rayAngle, x, hcurx, hcury, hcurGridX, hcurGridY, hDistance);

                                hcurx -= (horXa/2);
                                hcury -= (horYa/2);

                                thinWallDepth++;
                                thinWallHit = true;
                                // break; don't break, keep casting the ray
                        }
                        else // otherwise the ray passes through the door (it's opening/closing)
                        {
                            hcurx -= (horXa/2);
                            hcury -= (horYa/2);
                        }
                    }

                    // pretend nothing was found here, next time check next 
                    hcurx += (horXa);
                    hcury += (horYa);
                }
                else // This is a normal wall
                {
                    hDistance = fabs(sqrt((((player.centeredPos.x) - hcurx) * ((player.centeredPos.x) - hcurx)) + (((player.centeredPos.y) - hcury) * ((player.centeredPos.y) - hcury))));
                    hobjectIDHit = R_GetValueFromLevel(level, hcurGridY, hcurGridX);
                    horHitted = true;
                }
            }
            else
            {
                hDistance = FLT_MAX;
                horHitted = false;

                // nothing here, next time check next 
                hcurx += horXa;
                hcury += horYa;
            }
        }
        else // If the ray went outisde the map the check fails, stop
        {
            hDistance = FLT_MAX;
            horHitted = false;

            // nothing here, next time check next 
            hcurx += horXa;
            hcury += horYa;
            
            horEnded = true;
        }

        // ------------------
        // VERTICAL
        // ------------------
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
            int spriteID = R_GetValueFromSpritesMap(level, vcurGridY, vcurGridX);
            if(!visibleTiles[vcurGridY][vcurGridX])
            {
                // Add sprite and data in the array
                if(spriteID >= 1)
                R_AddToVisibleSprite(vcurGridX, vcurGridY, level, spriteID);

                // Check if there's an active dynamic sprite there
                if(G_GetFromDynamicSpriteMap(level, vcurGridY, vcurGridX) != NULL && G_GetFromDynamicSpriteMap(level, vcurGridY, vcurGridX)->base.active)
                    R_AddDynamicToVisibleSprite(level, vcurGridX, vcurGridY);

                // Check if there's an active dynamic sprite there
                if(G_GetFromDeadDynamicSpriteMap(level, vcurGridY, vcurGridX) != NULL && G_GetFromDeadDynamicSpriteMap(level, vcurGridY, vcurGridX)->base.active)
                    R_AddDeadDynamicToVisibleSprite(level, vcurGridX, vcurGridY);
            }

            int idHit = R_GetValueFromLevel(level, vcurGridY, vcurGridX);

            // If it hit a wall, register it, save the distance and get out of the while
            if(idHit >= 1)
            {
                // Check if this is a thin wall
                if(U_GetBit(&tomentdatapack.walls[idHit]->flags, 0) == 1)
                {
                    thinWallHit = true;

                        // Check if this is an Vertical thin wall (if it is an horizontal, just ignore it)
                    if(U_GetBit(&tomentdatapack.walls[idHit]->flags, 1) == 1)
                    {
                        // This is a thin wall, check if we hit it or if it was occluded by the wall

                        // Add half a tile to the current point
                        vcurx += (verXa/2);
                        vcury += (verYa/2);

                        // Calculate tile gird of the new point (added of a half a tile)
                        int newGridX = floor(vcurx / TILE_SIZE);
                        int newGridY = floor(vcury / TILE_SIZE);

                        // If the ray intersects with the door
                            // If they're in the same tile, the door is visible
                            if(newGridX == vcurGridX && newGridY == vcurGridY && thinWallDepth < MAX_THIN_WALL_TRANSPARENCY_RECURSION)
                            {
                                vDistance = fabs(sqrt((((player.centeredPos.x) - vcurx) * ((player.centeredPos.x) - vcurx)) + (((player.centeredPos.y) - vcury) * ((player.centeredPos.y) - vcury))));
                                
                                // Save the information about this hit, it will be drawn later after this ray draws a wall
                                I_AddThinWall(level, false, rayAngle, x, vcurx, vcury, vcurGridX, vcurGridY, vDistance);

                                vcurx -= (verXa/2);
                                vcury -= (verYa/2);

                                thinWallDepth++;
                                thinWallHit = true;
                                
                                // break; don't break, keep casting the ray
                            }
                            else // otherwise it's not visible, revert the point and keep scanning
                            {
                                vcurx -= (verXa/2);
                                vcury -= (verYa/2);
                            }
                    }

                    // pretend nothing was found here, next time check next 
                    vcurx += (verXa);
                    vcury += (verYa);
                }
                else
                {
                    vDistance = fabs(sqrt((((player.centeredPos.x) - vcurx) * ((player.centeredPos.x) - vcurx)) + (((player.centeredPos.y) - vcury) * ((player.centeredPos.y) - vcury))));
                    vobjectIDHit = R_GetValueFromLevel(level, vcurGridY, vcurGridX);
                    verHitted = true;
                }
            }
            else
            {
                vDistance = FLT_MAX;
                verHitted = false;

                // nothing here, next time check next 
                vcurx += verXa;
                vcury += verYa;
            }
        }
        else // If the ray went outisde the map the check fails, stop
        {
            vDistance = FLT_MAX;
            verHitted = false;

            // nothing here, next time check next 
            vcurx += verXa;
            vcury += verYa;

            verEnded = true;
        }

        // Check if there were hits
        // There's something to draw
        bool horizontal = false;    // Has this ray hit an horizontal?
        float correctDistance;      // Corrected distance for fixing fisheye
        
        // If the distances are the same we should skip, this happens very rarerly
        if(hDistance != vDistance && (verHitted || horHitted) && thinWallHit == false)
        {
            // If the horizontal hit was closer
            if(hDistance < vDistance && horHitted)
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
            else if (hDistance > vDistance && verHitted)
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

            // Add todraw
            walldata_t* data = &toDraw[toDrawLength];
            data->level = level;
            data->rayAngle = rayAngle;
            data->x = x;
            data->curX = wallPoint.x;
            data->curY = wallPoint.y;
            data->distance = correctDistance;
            data->gridPos.x = fcurGridX;
            data->gridPos.y = fcurGridY;
            data->idHit = objectIDHit;
            data->isVertical = !horizontal;

            // Increment the hor/ver distance in base of what we found, increment what we found so we don't check it two times
            if(horizontal)
            {
                hcurx += horXa;
                hcury += horYa;
            }
            else
            {
                vcurx += verXa;
                vcury += verYa;
            }
            
            toDrawLength++;


            //R_FloorCasting(end, rayAngle, x, wallHeight);
            //R_CeilingCasting(start, rayAngle, x, wallHeight);

            //R_DrawPixel(x, start, r_debugColor);
        }

        hDepth++;
        vDepth++;

        depth++;

        // If both hor and ver were outside of map, there's no point in going forward
        mapEndedOrMaxDistanceReached = (horEnded && verEnded && !thinWallHit);
    }

    // Order the todraw list from further to nearest (TODO: more optimized sort)
    walldata_t a;
    for (int i = 0; i < toDrawLength; ++i) 
        {
            for (int j = i + 1; j < toDrawLength; ++j) 
            {
                if (toDraw[i].distance < toDraw[j].distance) 
                {
                    a = toDraw[i];
                    toDraw[i] = toDraw[j];
                    toDraw[j] = a;
                }
            }
        }

    // Draw everything found in reverse order
    for(int tD = 0; tD < toDrawLength; tD++)
    {
        // Fix fisheye 
        float fixedAngle = rayAngle - player.angle;
        float finalDistance = toDraw[tD].distance * cos(fixedAngle);

        // DRAW WALL
        float wallHeight = (TILE_SIZE  / finalDistance) * DISTANCE_TO_PROJECTION;
        float wallHeightUncapped = wallHeight;

        float screenZ = floor(DISTANCE_TO_PROJECTION / finalDistance*(player.z-(TILE_SIZE/2)));
        int wallOffset = (PROJECTION_PLANE_CENTER) - floor(wallHeight / 2.0f) + screenZ;    // Wall Y offset to draw them in the middle of the screen + z

        int end = floor(wallOffset+wallHeight);
        int start = floor(wallOffset);

        float leveledStart = start-1 - floor(wallHeight)*level;
        float leveledEnd = end - floor(wallHeight)*level;

        // Check if start and end are offscreen, if so, don't draw the walls, but draw the bottom/top regardless
        bool isOffScreenBottom = (leveledStart > PROJECTION_PLANE_HEIGHT);
        if(leveledEnd > PROJECTION_PLANE_HEIGHT)
            leveledEnd = PROJECTION_PLANE_HEIGHT;

        //printf("LEVEL %1d - Start %6f - End %6f\n", level, start-1 - floor(wallHeight)*level, end - floor(wallHeight)*level+1);

        // Calculate lighting intensity
        float wallLighting = (PLAYER_POINT_LIGHT_INTENSITY + currentMap.wallLight)  / finalDistance;
        wallLighting = SDL_clamp(wallLighting, 0, 1.0f);

        object_t* curObject = tomentdatapack.walls[toDraw[tD].idHit];
        
        // Draw the walls as column of pixels
        if(!toDraw[tD].isVertical && !isOffScreenBottom) 
        {
            // Texture offset (shifted if it's a door)
            // Note:
            // For walls or closed doors the '- doorpos[]...' is always going to shift the curx 64.0 units
            // This is not a problem since it rounds back, shifting the offset by 64 puts us in the exact same place as if we never shifted, 
            // but this calculation lets us slide the texture offset too for the doors

            int offset = (int)(toDraw[tD].curX) % TILE_SIZE; 
            offset = SDL_clamp(offset, 0, TILE_SIZE);

            // If looking down, flip the texture offset
            if(rayAngle < M_PI)
                offset = (TILE_SIZE-1) - offset;
            
            R_DrawStripeTexturedShaded((x), leveledStart, leveledEnd, curObject->texture, offset, wallHeightUncapped, wallLighting, finalDistance);

            if(debugRendering)
            {
                SDL_Delay(10);
                SDL_UpdateWindowSurface(application.win);
            }
        }
        else if(!isOffScreenBottom)
        {
            // Texture offset (shifted if it's a door)
            // Note:
            // For walls or closed doors the '- doorpos[]...' is always going to shift the curx 64.0 units
            // This is not a problem since it rounds back, shifting the offset by 64 puts us in the exact same place as if we never shifted, 
            // but this calculation lets us slide the texture offse too for the doors
            int offset = (int)(toDraw[tD].curY) % TILE_SIZE;

            offset = SDL_clamp(offset, 0, TILE_SIZE);

            // If looking left, flip the texture offset
            if(rayAngle > M_PI / 2 && rayAngle < (3*M_PI) / 2)
                offset = (TILE_SIZE-1) - offset;

            R_DrawStripeTexturedShaded((x), leveledStart, leveledEnd, (curObject->alt != NULL) ? curObject->alt->texture : curObject->texture, offset, wallHeightUncapped, wallLighting, finalDistance);

            if(debugRendering)
            {
                SDL_Delay(10);
                SDL_UpdateWindowSurface(application.win);
            }
        }

        // Abs Floor casting
        if(level == 0)
            R_FloorCasting(end - floor(wallHeight)*level, rayAngle, x, wallHeight);

        // Draw bottom of walls
        float wallBottom = TILE_SIZE * level;
        int idBelow = -1;

        if(level > 0)
            idBelow = R_GetValueFromLevel(level-1, toDraw[tD].gridPos.y, toDraw[tD].gridPos.x);

        bool isEmptyBelow = (idBelow == 0);

        // If there's a door below this wall, we should draw the ceiling regardless
        if(!isEmptyBelow)
            if(idBelow != -1 && U_GetBit(&tomentdatapack.walls[idBelow]->flags, 2) == 1)
                isEmptyBelow = true;
        
        // Detect if we should draw the bottom/top face of this cube
        if(player.z < wallBottom && isEmptyBelow)
        {
            // Draw the bottom
            R_DrawWallBottom(&toDraw[tD], wallHeight, screenZ);
        }

        // Draw top of walls
        float wallTop = TILE_SIZE * (level+1);
        int idAbove = -1;

        if(level+1 < MAX_N_LEVELS)
            idAbove = R_GetValueFromLevel(level+1, toDraw[tD].gridPos.y, toDraw[tD].gridPos.x);

        bool isEmptyAbove = (idAbove == 0);

        // If there's a door below this wall, we should draw the ceiling regardless
        if(!isEmptyAbove)
            if(idAbove != -1 && U_GetBit(&tomentdatapack.walls[idAbove]->flags, 2) == 1)
                isEmptyAbove = true;

        // Detect if we should draw the bottom/top face of this cube
        if(player.z > wallTop && isEmptyAbove)
        {
            // Draw the bottom
            R_DrawWallTop(&toDraw[tD], wallHeight, screenZ);
        }
    }
}

void R_DrawWallBottom(walldata_t* wall, float height, float screenZ)
{
    //R_FloorCasting(bottom, wall->rayAngle, wall->x, height);
    int y = (PROJECTION_PLANE_HEIGHT - height) / 2 + height;
    y = y - height*wall->level + screenZ;

    if(y < 0)
        y = 0;

    //R_DrawPixel(wall->x ,y, r_debugColor); 

    float beta = (player.angle - wall->rayAngle);
    FIX_ANGLES(beta);

    bool startedDrawing = false;
    for(; y < PROJECTION_PLANE_CENTER; y++)
    {
        float ceilingHeight = TILE_SIZE * wall->level;

        // Get distance
        float straightlinedist = (((ceilingHeight - player.z) * DISTANCE_TO_PROJECTION) / (PROJECTION_PLANE_CENTER-y));
        float d = straightlinedist / cos(beta);

        // Get coordinates
        float floorx = player.centeredPos.x + (cos(wall->rayAngle) * d);
        float floory = player.centeredPos.y + (sin(wall->rayAngle) * d);

        // Get map coordinates
        int curGridX = floor(floorx / TILE_SIZE);
        int curGridY = floor(floory / TILE_SIZE);

        bool isInWall = curGridX==wall->gridPos.x && curGridY==wall->gridPos.y;
        
        int wallID = R_GetValueFromLevel(wall->level, curGridY, curGridX); 
        
        // If wall ended
        if(wallID == 0 || !isInWall)
        {
            if(startedDrawing)
                return;
            else
                continue;
        }

        // Get textels
        int textureX = (int)floorx % TILE_SIZE;
        int textureY = (int)floory % TILE_SIZE;

        // Calculate lighting intensity
        float lightingMult = (PLAYER_POINT_LIGHT_INTENSITY + currentMap.floorLight) / d;
        lightingMult = SDL_clamp(lightingMult, 0, 1.0f);

        // If the ray is in a grid that is inside the map
        if(curGridX >= 0 && curGridY >= 0 && curGridX < MAP_WIDTH && curGridY < MAP_HEIGHT)
        {
            // Check the floor texture at that point
            if(wallID >= 1)
            {
                // Draw floor
                R_DrawColumnOfPixelShaded(wall->x, y-1, y+1, R_GetPixelFromSurface(*tomentdatapack.walls[wallID]->bottomTexture, textureX, textureY), lightingMult, straightlinedist-1.0f);

                startedDrawing = true;
            }
        }
    }
}

void R_DrawWallTop(walldata_t* wall, float height, float screenZ)
{
    float beta = (player.angle - wall->rayAngle);
    FIX_ANGLES(beta);

    int y = (PROJECTION_PLANE_HEIGHT - height) / 2;
    y = y - height*wall->level + screenZ;
    
    if(y > PROJECTION_PLANE_HEIGHT)
        y = PROJECTION_PLANE_HEIGHT;

    //R_DrawPixel(wall->x ,y, r_debugColor); 

    float wallTop = (wall->level+1)*TILE_SIZE;

    bool startedDrawing = false;
    for(; y >= PROJECTION_PLANE_CENTER; y--)
    {
        // Get distance
        float straightlinedist = ((player.z - wallTop) * DISTANCE_TO_PROJECTION) / (y - PROJECTION_PLANE_CENTER);
        float d = straightlinedist / cos(beta);

        // Get coordinates
        float floorx = player.centeredPos.x + (cos(wall->rayAngle) * d);
        float floory = player.centeredPos.y + (sin(wall->rayAngle) * d);

        // Get map coordinates
        int curGridX = floor(floorx / TILE_SIZE);
        int curGridY = floor(floory / TILE_SIZE);

        bool isInWall = curGridX==wall->gridPos.x && curGridY==wall->gridPos.y;

        int wallID = R_GetValueFromLevel(wall->level, curGridY, curGridX);
        // If wall ended
        if(wallID == 0 || !isInWall)
        {
            if(startedDrawing)
                return;
            else
                continue;
        }

        // Get textels
        int textureX = (int)floorx % TILE_SIZE;
        int textureY = (int)floory % TILE_SIZE;
        
        // Calculate lighting intensity
        float floorLighting = (PLAYER_POINT_LIGHT_INTENSITY + currentMap.floorLight) / d;
        floorLighting = SDL_clamp(floorLighting, 0, 1.0f);

        // If the ray is in a grid that is inside the map
        if(curGridX >= 0 && curGridY >= 0 && curGridX < MAP_WIDTH && curGridY < MAP_HEIGHT)
        {
            // Check the floor texture at that point
            if(wallID >= 1)
            {                
                // Draw floor
                R_DrawPixelShaded(wall->x, y, R_GetPixelFromSurface(*tomentdatapack.walls[wallID]->topTexture, textureX, textureY), floorLighting, straightlinedist-1.0f);

                startedDrawing = true;
            }
        }
    }
}

//-------------------------------------
// Floorcast and ceilingcast
// Params:
// - end = the end of the wall that states where to start to floorcast
// - rayAngle = the current rayangle
// - x = the x coordinate on the screen for this specific floor cast call
//-------------------------------------
void R_FloorCasting(float end, float rayAngle, int x, float wallHeight)
{
    if(end > PROJECTION_PLANE_HEIGHT)
        end = PROJECTION_PLANE_HEIGHT;

    // Floor Casting & Ceiling
    float beta = (player.angle - rayAngle);
    FIX_ANGLES(beta);

    for(int y = floor(end); y < PROJECTION_PLANE_HEIGHT; y++)
    {
        // Get distance
        float straightlinedist = (player.z * DISTANCE_TO_PROJECTION) / (y - PROJECTION_PLANE_CENTER);
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
                R_DrawPixelShaded(x, y, R_GetPixelFromSurface(tomentdatapack.floors[floorObjectID]->texture, textureX, textureY), floorLighting, d);
            }
        }
    }
}

void R_CeilingCasting(int level, float start, float rayAngle, int x, float wallHeight)
{
    // Floor Casting & Ceiling
    float beta = (player.angle - rayAngle);
    FIX_ANGLES(beta);
    float ceilingHeight = TILE_SIZE * (level+1);
    // If the current ceiling height is greater than 1, ceiling needs to be calculated on its own
    for(int y = floor(start); y >= 0; y--)
    {
        // Get distance
        float straightlinedist = (((ceilingHeight - player.z)* DISTANCE_TO_PROJECTION) / (PROJECTION_PLANE_CENTER-y));
        float d = straightlinedist / cos(beta);

        // Get coordinates
        float floorx = player.centeredPos.x + (cos(rayAngle) * d);
        float floory = player.centeredPos.y + (sin(rayAngle) * d);

        // Get map coordinates
        int curGridX = floor(floorx / TILE_SIZE);
        int curGridY = floor(floory / TILE_SIZE);

        straightlinedist = (((ceilingHeight - player.z)* DISTANCE_TO_PROJECTION) / (PROJECTION_PLANE_CENTER-y));
        d = straightlinedist / cos(beta);

        // Calculate lighting intensity
        float floorLighting = (PLAYER_POINT_LIGHT_INTENSITY + currentMap.floorLight) / d;
        floorLighting = SDL_clamp(floorLighting, 0, 1.0f);

        // Get coordinates
        floorx = player.centeredPos.x + (cos(rayAngle) * d);
        floory = player.centeredPos.y + (sin(rayAngle) * d);

        // Get textels
        int textureX = (int)floorx % TILE_SIZE;
        int textureY = (int)floory % TILE_SIZE;

        // Get map coordinates
        curGridX = floor(floorx / TILE_SIZE);
        curGridY = floor(floory / TILE_SIZE);

        int floorObjectID = -1;
        int ceilingObjectID = -1;

        // If the ray is in a grid that is inside the map
        if(curGridX >= 0 && curGridY >= 0 && curGridX < MAP_WIDTH && curGridY < MAP_HEIGHT)
        {
            if(currentMap.ceilingMap[curGridY][curGridX] >= 1)
            {
                ceilingObjectID = currentMap.ceilingMap[curGridY][curGridX];

                // Draw ceiling
                R_DrawPixelShaded(x, (y), R_GetPixelFromSurface(tomentdatapack.ceilings[ceilingObjectID]->texture, textureX, textureY), floorLighting, d);
            }
        }
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

    float screenZ = round(DISTANCE_TO_PROJECTION / finalDistance*(player.z-(TILE_SIZE/2)));
    
    int wallOffset = (PROJECTION_PLANE_CENTER) - floor(wallHeight / 2.0f) + screenZ;    // Wall Y offset to draw them in the middle of the screen + z

    float start = floor(wallOffset);
    float end = floor(wallOffset+wallHeight);

    float leveledStart = start-(wallHeight*cur->level);
    float leveledEnd = end-(wallHeight*cur->level);

    // Check if start and end are offscreen, if so, don't draw the walls, but draw the bottom/top regardless
    bool isOffScreenBottom = (leveledStart > PROJECTION_PLANE_HEIGHT);
    if(leveledEnd > PROJECTION_PLANE_HEIGHT)
        leveledEnd = PROJECTION_PLANE_HEIGHT;

    // Calculate lighting intensity
    float wallLighting = (PLAYER_POINT_LIGHT_INTENSITY + currentMap.wallLight)  / finalDistance;
    wallLighting = SDL_clamp(wallLighting, 0, 1.0f);

    object_t* curObject = tomentdatapack.walls[cur->idHit];

    // Draw the walls as column of pixels
    if(!cur->isVertical && !isOffScreenBottom) 
    {
        // Texture offset (shifted if it's a door)
        // Note:
        // For walls or closed doors the '- doorpos[]...' is always going to shift the curx 64.0 units
        // This is not a problem since it rounds back, shifting the offset by 64 puts us in the exact same place as if we never shifted, 
        // but this calculation lets us slide the texture offset too for the doors
        int offset = (int)(cur->curX - (G_GetDoorPosition(cur->level, cur->gridPos.y, cur->gridPos.x))) % TILE_SIZE;

        // If looking down, flip the texture offset
        if(cur->rayAngle < M_PI)
            offset = (TILE_SIZE-1) - offset;
        
        if(cur->extraData == 1) // If it is visible
            R_DrawStripeTexturedShaded((cur->x), leveledStart, leveledEnd, curObject->texture, offset, wallHeightUncapped, wallLighting, finalDistance);
    }
    else if(!isOffScreenBottom)
    {
        // Texture offset (shifted if it's a door)
        // Note:
        // For walls or closed doors the '- doorpos[]...' is always going to shift the curx 64.0 units
        // This is not a problem since it rounds back, shifting the offset by 64 puts us in the exact same place as if we never shifted, 
        // but this calculation lets us slide the texture offse too for the doors
        int offset = (int)(cur->curY - (G_GetDoorPosition(cur->level, cur->gridPos.y, cur->gridPos.x))) % TILE_SIZE;

        // If looking left, flip the texture offset
        if(cur->rayAngle > M_PI / 2 && cur->rayAngle < (3*M_PI) / 2)
            offset = (TILE_SIZE-1) - offset;

        if(cur->extraData == 1) // If it is visible
            R_DrawStripeTexturedShaded((cur->x), leveledStart, leveledEnd, (curObject->alt != NULL) ? curObject->alt->texture : curObject->texture, offset, wallHeightUncapped, wallLighting, finalDistance);
    }
}


//-------------------------------------
// Adds a sprite to the visible sprite array and adds its corresponding drawable
//-------------------------------------
void R_AddToVisibleSprite(int gridX, int gridY, int level, int spriteID)
{
     // Check if it's a dynamic
    if(U_GetBit(&tomentdatapack.sprites[spriteID]->flags, 2) == 1)
        return;

    visibleSprites[visibleSpritesLength].level = level;

    // Save Grid Pos
    visibleSprites[visibleSpritesLength].gridPos.x = gridX;
    visibleSprites[visibleSpritesLength].gridPos.y = gridY;

    // Get World Pos
    visibleSprites[visibleSpritesLength].pos.x = gridX * TILE_SIZE;
    visibleSprites[visibleSpritesLength].pos.y = gridY * TILE_SIZE;

    visibleSprites[visibleSpritesLength].centeredPos.x = visibleSprites[visibleSpritesLength].pos.x + (TILE_SIZE / 2);
    visibleSprites[visibleSpritesLength].centeredPos.y = visibleSprites[visibleSpritesLength].pos.y + (TILE_SIZE / 2);

    // Get Player Space pos
    visibleSprites[visibleSpritesLength].pSpacePos.x = visibleSprites[visibleSpritesLength].centeredPos.x - player.centeredPos.x;
    visibleSprites[visibleSpritesLength].pSpacePos.y = visibleSprites[visibleSpritesLength].centeredPos.y - player.centeredPos.y;

    // Calculate the distance to player
    visibleSprites[visibleSpritesLength].dist = sqrt(visibleSprites[visibleSpritesLength].pSpacePos.x*visibleSprites[visibleSpritesLength].pSpacePos.x + visibleSprites[visibleSpritesLength].pSpacePos.y*visibleSprites[visibleSpritesLength].pSpacePos.y);

    // Get ID
    visibleSprites[visibleSpritesLength].spriteID = spriteID;
    visibleSprites[visibleSpritesLength].sheetLength = tomentdatapack.spritesSheetsLenghtTable[spriteID];

    // Sprite is also a drawable
    // Add it to the drawables
    allDrawables[allDrawablesLength].type = DRWB_SPRITE;
    allDrawables[allDrawablesLength].spritePtr = &visibleSprites[visibleSpritesLength];
    
    // Quick variable access
    allDrawables[allDrawablesLength].dist = visibleSprites[visibleSpritesLength].dist;
    
    // Increment indexes
    allDrawablesLength++;
    visibleSpritesLength++;

    
    // Mark this sprite as added so we don't get duplicates
    visibleTiles[gridY][gridX] = true;
}

void R_AddDynamicToVisibleSprite(int level, int gridX, int gridY)
{
    // Check if it's a dynamic
    dynamicSprite_t* dynamicSprite = G_GetFromDynamicSpriteMap(level, gridY, gridX);

    // Sprite is also a drawable
    // Add it to the drawables
    allDrawables[allDrawablesLength].type = DRWB_DYNAMIC_SPRITE;
    allDrawables[allDrawablesLength].dynamicSpritePtr = G_GetFromDynamicSpriteMap(level, gridY, gridX);
    
    // Quick variable access
    allDrawables[allDrawablesLength].dist = dynamicSprite->base.dist;
    
    // Increment indexes
    allDrawablesLength++;

    // Mark this sprite as added so we don't get duplicates
    visibleTiles[gridY][gridX] = true;
}

void R_AddDeadDynamicToVisibleSprite(int level, int gridX, int gridY)
{
    // Check if it's a dynamic
    dynamicSprite_t* dynamicSprite = G_GetFromDeadDynamicSpriteMap(level, gridY, gridX);

    // Sprite is also a drawable
    // Add it to the drawables
    allDrawables[allDrawablesLength].type = DRWB_DYNAMIC_SPRITE;
    allDrawables[allDrawablesLength].dynamicSpritePtr = G_GetFromDeadDynamicSpriteMap(level, gridY, gridX);
    
    // Quick variable access
    allDrawables[allDrawablesLength].dist = dynamicSprite->base.dist;
    
    // Increment indexes
    allDrawablesLength++;

    // Mark this sprite as added so we don't get duplicates
    visibleTiles[gridY][gridX] = true;
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

    float screenZ = round(DISTANCE_TO_PROJECTION / dist*(player.z-(TILE_SIZE/2)));
    int spriteOffset = (PROJECTION_PLANE_CENTER) - floor(sprite->height / 2.0f) + screenZ;    // Wall Y offset to draw them in the middle of the screen + z

    if(sprite->height <= 0)
        return;

    if(sprite->height > MAX_SPRITE_HEIGHT)
        sprite->height = MAX_SPRITE_HEIGHT;

    // Calculate lighting intensity
    float lighting = (PLAYER_POINT_LIGHT_INTENSITY + currentMap.floorLight) / dist;
    lighting = SDL_clamp(lighting, 0, 1.0f);

    // Draw
    int offset, drawX, drawYStart, drawYEnd, currentFrame = 0;

    if(sprite->sheetLength > 0 && U_GetBit(&tomentdatapack.sprites[sprite->spriteID]->flags, 1) == 1)
        currentFrame = ((int)floor(curTime / ANIMATION_SPEED_DIVIDER) % sprite->sheetLength);

    for(int j = 0; j < sprite->height; j++)
    {
        offset = j*TILE_SIZE/sprite->height + (UNIT_SIZE*currentFrame);
        drawX = PROJECTION_PLANE_WIDTH-(spriteX)+j-(sprite->height/2);

        drawYStart = spriteOffset;
        drawYEnd = spriteOffset+sprite->height;

        R_DrawStripeTexturedShaded(drawX, drawYStart-(sprite->height*sprite->level), drawYEnd-(sprite->height*sprite->level), tomentdatapack.sprites[sprite->spriteID]->texture,offset, sprite->height, lighting, dist);
    }

    // Draws the center of the sprite
    //R_DrawPixel(PROJECTION_PLANE_WIDTH-spriteX, spriteY, SDL_MapRGB(win_surface->format, 255, 0, 0));
}

//-------------------------------------
// Draws the passed sprite
//-------------------------------------
void R_DrawDynamicSprite(dynamicSprite_t* sprite)
{
    // Done in degrees to avoid computations (even if I could cache radians values and stuff)
    // Calculate angle and convert to degrees (*-1 makes sure it uses SDL screen space coordinates for unit circle and quadrants)
    float angle;

    // AI should always face the player, projectiles should be drawn like normal sprites
    if(sprite->type == DS_TYPE_AI)
        angle = sprite->base.angle = ((atan2(-sprite->base.pSpacePos.y, sprite->base.pSpacePos.x))* RADIAN_TO_DEGREE)*-1;
    else
        angle = ((atan2(-sprite->base.pSpacePos.y, sprite->base.pSpacePos.x))* RADIAN_TO_DEGREE)*-1;

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
    float dist = (sprite->base.dist * cos(fixedAngle));

    sprite->base.height = DISTANCE_TO_PROJECTION * TILE_SIZE / dist;

    float screenZ = round(DISTANCE_TO_PROJECTION / dist*(player.z-(TILE_SIZE/2)));
    int spriteOffset = (PROJECTION_PLANE_CENTER) - floor(sprite->base.height / 2.0f) + screenZ;    // Wall Y offset to draw them in the middle of the screen + z

    if(sprite->base.height <= 0)
        return;

    if(sprite->base.height > MAX_SPRITE_HEIGHT)
        sprite->base.height = MAX_SPRITE_HEIGHT;

    // Calculate lighting intensity
    float lighting = (PLAYER_POINT_LIGHT_INTENSITY + currentMap.floorLight) / dist;
    lighting = SDL_clamp(lighting, 0, 1.0f);

    // Draw
    int offset, drawX, drawYStart, drawYEnd;

    // Select Animation
    SDL_Surface* curAnim;
    int curAnimLength = 0;

    switch(sprite->state)
    {
        case DS_STATE_IDLE:
            curAnim = tomentdatapack.sprites[sprite->base.spriteID]->animations->animIdle;
            curAnimLength = tomentdatapack.sprites[sprite->base.spriteID]->animations->animIdleSheetLength;
            break;

        case DS_STATE_DEAD:
            curAnim = tomentdatapack.sprites[sprite->base.spriteID]->animations->animDie;
            curAnimLength = tomentdatapack.sprites[sprite->base.spriteID]->animations->animDieSheetLength;
            break;

        case DS_STATE_ATTACKING:
            curAnim = tomentdatapack.sprites[sprite->base.spriteID]->animations->animAttack;
            curAnimLength = tomentdatapack.sprites[sprite->base.spriteID]->animations->animAttackSheetLength;
            break;

        default:
            curAnim = tomentdatapack.sprites[sprite->base.spriteID]->animations->animIdle;
            curAnimLength = tomentdatapack.sprites[sprite->base.spriteID]->animations->animIdleSheetLength;
            break;
    }

    if(sprite->animPlay)
    {
        if(sprite->animPlayOnce)
        {
            if(curAnimLength > 0)
                sprite->animFrame = ((int)floor(sprite->animTimer->GetTicks(sprite->animTimer) / sprite->animSpeed) % curAnimLength);

            // Prevent loop
            if(sprite->animFrame >= curAnimLength-1)
            {
                sprite->animPlay = false;

                // Go back to idle
                if(sprite->state == DS_STATE_ATTACKING)
                {
                    G_AIPlayAnimationLoop(sprite, ANIM_IDLE);
                }
            }
        }
        else
        {
            // Allow loop
             if(curAnimLength > 0)
                sprite->animFrame = ((int)floor(sprite->animTimer->GetTicks(sprite->animTimer) / sprite->animSpeed) % curAnimLength);
        }
    }

    for(int j = 0; j < sprite->base.height; j++)
    {
        offset = j*TILE_SIZE/sprite->base.height + (UNIT_SIZE*sprite->animFrame);
        drawX = PROJECTION_PLANE_WIDTH-(spriteX)+j-(sprite->base.height/2);

        drawYStart = spriteOffset;
        drawYEnd = spriteOffset+sprite->base.height;

        R_DrawStripeTexturedShaded(drawX, drawYStart-(sprite->base.height*sprite->base.level), drawYEnd-(sprite->base.height*sprite->base.level), curAnim,offset, sprite->base.height, lighting, dist);
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

    int counter = 0;
    for(int i = 0; i < allDrawablesLength; i++)
    {
        switch(allDrawables[i].type)
        {
            case DRWB_WALL:
                R_DrawThinWall(allDrawables[i].wallPtr);
                break;

            case DRWB_SPRITE:
                counter++;
                R_DrawSprite(allDrawables[i].spritePtr);
                break;

            case DRWB_DYNAMIC_SPRITE:
                R_DrawDynamicSprite(allDrawables[i].dynamicSpritePtr);
                break;
        }
    }

    projectileNode_t* cur = projectilesHead;
    while(cur != NULL)
    {
        R_DrawDynamicSprite(&cur->this);
        cur = cur->next;
    }

    //printf("DRAWN %d\n", counter);
}


int R_GetValueFromLevel(int level, int y, int x)
{
    if(x >= 0 && y >= 0 && x < MAP_WIDTH && y < MAP_HEIGHT)
    {
        switch(level)
        {
            case 0:
                return currentMap.level0[y][x];

            case 1:
                return currentMap.level1[y][x];

            case 2:
                return currentMap.level2[y][x];

            default:
                //printf("WARNING! Level get was out of max/min level size\n");
                return 0;
        }
    }
    else
    {
        return 0;
    }
}

int R_GetValueFromSpritesMap(int level, int y, int x)
{
    if(x >= 0 && y >= 0 && x < MAP_WIDTH && y < MAP_HEIGHT)
    {
        switch(level)
        {
            case 0:
                return currentMap.spritesMapLevel0[y][x];

            case 1:
                return currentMap.spritesMapLevel1[y][x];

            case 2:
                return currentMap.spritesMapLevel2[y][x];

            default:
                //printf("WARNING! Level get was out of max/min level size\n");
                return 0;
        }
    }
    else
    {
        return 0;
    }
}

void R_SetValueFromSpritesMap(int level, int y, int x, int value)
{
    if(x >= 0 && y >= 0 && x < MAP_WIDTH && y < MAP_HEIGHT)
    {
        switch(level)
        {
            case 0:
                currentMap.spritesMapLevel0[y][x] = value;
                break;

            case 1:
                currentMap.spritesMapLevel1[y][x] = value;
                break;

            case 2:
                currentMap.spritesMapLevel2[y][x] = value;
                break;

            default:
                //printf("WARNING! Level get was out of max/min level size\n");
               break;
        }
    }
}

//-------------------------------------
// Gets a pixel from a surface
//-------------------------------------
Uint32 R_GetPixelFromSurface(SDL_Surface *surface, int x, int y)
{
    Uint32* target_pixel = (Uint32 *) ((Uint8 *) surface->pixels
                        + y * surface->pitch
                        + x * surface->format->BytesPerPixel);

    return *target_pixel;
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
void R_BlitIntoScreen(SDL_Rect* size, SDL_Surface* sur, SDL_Rect* pos)
{
    SDL_BlitSurface(sur, size, win_surface, pos);
}

//-------------------------------------
// Given an SDL_Surface, extracts the pixels of it and puts them in the selected framebuffer
//-------------------------------------
void R_BlitIntoScreenScaled(SDL_Rect* size, SDL_Surface* sur, SDL_Rect* pos)
{
    SDL_BlitScaled(sur, size, win_surface, pos);
}

//-------------------------------------
// Given an SDL_Surface, extracts the pixels of it and puts them in the selected framebuffer
//-------------------------------------
void R_BlitColorIntoScreen(int color, SDL_Rect* pos)
{
    for(int y = 0; y < pos->h; y++)
        for(int x = 0; x < pos->w; x++) 
        {
            // Put it into buffer
            pixels[(pos->x + x) + ((pos->y + y) * SCREEN_WIDTH)] = color;
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
            pixels[x + y * win_width] = color;
        if (D > 0)
        {
            y = y + yi;
            D = D + (2 * (dy - dx));
        }
        else
            D = D + 2 * dy;
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
            pixels[x + y * win_width] = color;
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
            pixels[x + y * win_width] = color;
}

//------------------------------------------------
// Draws a single pixel on the current framebuffer
//-------------------------------------------------
void R_DrawPixelShaded(int x, int y, int color, float intensity, float dist)
{
    if( x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT)    // To not go outside of boundaries
    {
        // Put it in the framebuffer
        // Put in Z buffer
        if(dist < zBuffer[y][x])
        {
            // Do shading
            Uint8 r,g,b;
            SDL_GetRGB(color, win_surface->format, &r, &g, &b);
            r*=intensity;
            g*=intensity;
            b*=intensity;
        
            zBuffer[y][x] = dist;
            pixels[x + y * win_width] = SDL_MapRGB(win_surface->format, r,g,b);
        }
    }
}

//-------------------------------------
// Draw a column of pixels with shading
//-------------------------------------
void R_DrawColumnOfPixelShaded(int x, int y, int endY, int color, float intensity, float distance)
{
    Uint32 pixel;

    // Do shading
    Uint8 r,g,b;
    SDL_GetRGB(color, win_surface->format, &r, &g, &b);
    r*=intensity;
    g*=intensity;
    b*=intensity;

    pixel = SDL_MapRGB(win_surface->format, r,g,b);

    for(int i = y; i <= endY; i++)
    {
        if( x >= 0 && x < SCREEN_WIDTH && i >= 0 && i < SCREEN_HEIGHT)    // To not go outside of boundaries
        {
            // Put it in the framebuffer
            // Put in Z buffer
            if(distance < zBuffer[i][x])
            {
                zBuffer[i][x] = distance;
                pixels[x + i * win_width] = pixel;
            }
        }
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
            pixels[x + i * win_width] = color;
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
        // Put it in the framebuffer
        if(x < PROJECTION_PLANE_WIDTH && x >= 0 && i < PROJECTION_PLANE_HEIGHT && i >= 0) // Don't overflow
        {
            // Extract pixel
            Uint32 pixel = R_GetPixelFromSurface(texture, xOffset, textureY);

            // Update the Z buffer;
             if(pixel != r_transparencyColor)
                if(wallheight < zBuffer[i][x])
                {
                    zBuffer[i][x] = wallheight; 
                    pixels[x + i * win_width] = pixel;
                }
        }

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
void R_DrawStripeTexturedShaded(int x, int y, int endY, SDL_Surface* texture, int xOffset, float wallheight, float intensity, float dist)
{
    // The offset to extract Y pixels from the texture, this is != 0 only if the wall is bigger than the projection plane height
    float textureYoffset = 0.0f;

    // Increment at each pixel
    float offset = TILE_SIZE / (wallheight);

    // The actual Y index
    float textureY = textureYoffset * offset;
    for(int i = y; i < endY; i++)
    {
        if(x < PROJECTION_PLANE_WIDTH && x >= 0 && i < PROJECTION_PLANE_HEIGHT && i >= 0) // Don't overflow
        {
            // Extract pixel
            Uint32 pixel = R_GetPixelFromSurface(texture, xOffset, textureY);

            if(dist < zBuffer[i][x] && pixel != r_transparencyColor)
            {
                // Do shading
                Uint8 r,g,b;
                SDL_GetRGB(pixel, texture->format, &r, &g, &b);
                r*=intensity;
                g*=intensity;
                b*=intensity;

                // Put it in the framebuffer

                // Update the Z buffer;
                pixels[x + i * win_width] = SDL_MapRGB(texture->format, r,g,b);
                zBuffer[i][x] = dist; 
            }
        }

        // Go forward
        textureY+= offset;
    }
}


// Save the information about this hit, it will be drawn later after this ray draws a wall
void I_AddThinWall(int level, bool horizontal, float rayAngle, int x, float curX, float curY, int gridX, int gridY, float distance)
{
    if(visibleThinWallsLength >= PROJECTION_PLANE_WIDTH * MAX_THIN_WALL_TRANSPARENCY_RECURSION)
    {
        printf("problem");
        return;
    }
    walldata_t* data = &currentThinWalls[visibleThinWallsLength];
    data->level = level;
    data->rayAngle = rayAngle;
    data->x = x;
    data->curX = curX;
    data->curY = curY;
    data->distance = distance;
    data->gridPos.x = gridX;
    data->gridPos.y = gridY;
    data->idHit = R_GetValueFromLevel(level, gridY, gridX);
    data->isVertical = !horizontal;

    if(horizontal)
        data->extraData = (curX - (UNIT_SIZE*gridX)) < G_GetDoorPosition(data->level, gridY, gridX); // Is Door Visible (we have to ceil cast even if the door is closed, but we don't have to show the actual door if it is closed)
    else
        data->extraData = (curY - (UNIT_SIZE*gridY)) < G_GetDoorPosition(data->level, gridY, gridX);

    // Add it to the drawables
    allDrawables[allDrawablesLength].type = DRWB_WALL;
    allDrawables[allDrawablesLength].wallPtr = &currentThinWalls[visibleThinWallsLength];
    // Quick variable access
    allDrawables[allDrawablesLength].dist = data->distance;

    allDrawablesLength++;
    visibleThinWallsLength++;
}

static void I_DebugPathfinding(void)
{
    int level = (allDynamicSprites[0] != NULL) ? allDynamicSprites[0]->base.level : player.level;

    SDL_Rect curRect;

    // If the map has never been drawn, draw it in the framebuffer 1
    for(int y = 0; y < MAP_HEIGHT; y++)
        for(int x = 0; x < MAP_WIDTH; x++)
        {
            // Set X and Y
            curRect.w = TILE_SIZE / MINIMAP_DIVIDER;
            curRect.h = TILE_SIZE / MINIMAP_DIVIDER;
            curRect.x = x * TILE_SIZE / MINIMAP_DIVIDER;
            curRect.y = y * TILE_SIZE / MINIMAP_DIVIDER;

            // If it is an empty space
            if(R_GetValueFromLevel(level, y, x) == 0)
            {
                R_BlitColorIntoScreen(SDL_MapRGB(win_surface->format, 0, 0, 0), &curRect);
            }
            else
            {
                R_BlitColorIntoScreen(SDL_MapRGB(win_surface->format, 255, 0, 0), &curRect);
            }
        }
}