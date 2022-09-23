#include "G_Pathfinding.h"
#include "R_Rendering.h"
#include "../include/SDL2/SDL.h"
#include "G_Player.h"

pathnode_t frontier[MAP_HEIGHT*MAP_WIDTH+2];
unsigned int frontierLength = 0;
bool visited[MAP_HEIGHT][MAP_WIDTH];

static void I_InsertNode(int level, pathnode_t* node, int gridx, int gridy, int f, int g, int h, pathnode_t* parent, bool debug, sprite_t* entity);
static void I_AddAdiacentNodes(int level, int oGridX, int oGridY, pathnode_t* parent, bool debug, sprite_t* entity);


path_t G_PerformPathfinding(int level, vector2Int_t gridPos, vector2Int_t gridTargetPos, sprite_t* entity) //entity performing pathfinding
{
    // Path to return
    path_t path;

    bool pathExists = false;

    // Reset state
    memset(visited, false, MAP_HEIGHT*MAP_WIDTH*sizeof(bool));
    frontierLength = 0;
    
    // Insert the start node and its adiacents
    I_InsertNode(level, &frontier[0], gridPos.x, gridPos.y, 0, 0, 0, NULL, false, entity);
    I_AddAdiacentNodes(level, gridPos.x, gridPos.y, &frontier[0], false, entity);

    // Check element keeps track of the elements that have been checked
    int checkElement = 1;
    
    // While there are elements that haven't been checked
    while(checkElement < frontierLength)
    {
        // Get that element
        pathnode_t* cur = &frontier[checkElement];

        // Check If the target was found
        if(cur->gridPos.x == gridTargetPos.x && cur->gridPos.y == gridTargetPos.y)
        {
            // If so, path exists
            pathExists = true;
            break;
        }

        // Otherwise we'll check all the other adiacent nodes
        I_AddAdiacentNodes(level, cur->gridPos.x, cur->gridPos.y, cur, false, entity);
        checkElement++;
    }

    // Once the BFS ended, if the target was found
    if(pathExists)
    {
        // Create the path
        path.isValid = true;
        pathnode_t* cur;

        // Last element of frontier is the goal, which is the first element of the path (path is reversed) 
        cur = &frontier[checkElement];
        path.nodes[0] = cur;
        path.nodesLength = 1;
        
        // Track back to the entity who performed the pathfinding
        while((cur = cur->parent) != &frontier[0])
        {
            path.nodes[path.nodesLength] = cur;
            path.nodesLength++;
        }
        
        // Return it
        return path;
    }
    else
    {
        // Path is invalid
        path.isValid = false;
        return path;
    }
}

path_t G_PerformPathfindingDebug(int level, vector2Int_t gridPos, vector2Int_t gridTargetPos)
{
    // Path to return
    path_t path;

    bool pathExists = false;

    // Reset state
    memset(visited, false, MAP_HEIGHT*MAP_WIDTH*sizeof(bool));
    frontierLength = 0;
    
    // Insert the start node and its adiacents
    I_InsertNode(level, &frontier[0], gridPos.x, gridPos.y, 0, 0, 0, NULL, true, NULL);
    I_AddAdiacentNodes(level, gridPos.x, gridPos.y, &frontier[0], true, NULL);

    // Draw start
    SDL_Rect curRect;
    curRect.w = TILE_SIZE / MINIMAP_DIVIDER;
    curRect.h = TILE_SIZE / MINIMAP_DIVIDER;
    curRect.x = gridPos.x * TILE_SIZE / MINIMAP_DIVIDER;
    curRect.y = gridPos.y * TILE_SIZE / MINIMAP_DIVIDER;
    R_BlitColorIntoScreen(SDL_MapRGB(win_surface->format, 0, 0, 255), &curRect);

    // Check element keeps track of the elements that have been checked
    int checkElement = 1;
    
    // While there are elements that haven't been checked
    while(checkElement < frontierLength)
    {
        // Get that element
        pathnode_t* cur = &frontier[checkElement];

        // Check If the target was found
        if(cur->gridPos.x == gridTargetPos.x && cur->gridPos.y == gridTargetPos.y)
        {
            // If so, path exists
            pathExists = true;

            // Draw the path
            // Set X and Y
            curRect.w = TILE_SIZE / MINIMAP_DIVIDER;
            curRect.h = TILE_SIZE / MINIMAP_DIVIDER;
            curRect.x = gridTargetPos.x * TILE_SIZE / MINIMAP_DIVIDER;
            curRect.y = gridTargetPos.y * TILE_SIZE / MINIMAP_DIVIDER;
            // If it is an empty space
            R_BlitColorIntoScreen(SDL_MapRGB(win_surface->format, 255, 255, 0), &curRect);
            SDL_UpdateWindowSurface(application.win);
            SDL_Delay(2000);

            // Stop looking
            break;
        }

        // Otherwise we'll check all the other adiacent nodes
        I_AddAdiacentNodes(level, cur->gridPos.x, cur->gridPos.y, cur, true, NULL);
        checkElement++;
    }

    // Once the BFS ended, if the target was found
    if(pathExists)
    {
        // Create the path
        path.isValid = true;
        pathnode_t* cur;

        // Last element of frontier is the goal, which is the first element of the path (path is reversed) 
        cur = &frontier[checkElement];
        path.nodes[0] = cur;
        int pathLength = 1;
        
        // Track back to the entity who performed the pathfinding
        while((cur = cur->parent) != &frontier[0])
        {
            path.nodes[pathLength] = cur;
            pathLength++;
        }

        // There you have it
        printf("Path length: %d\n", pathLength);

        // Draw path
        for(int i = 0; i < pathLength; i++)
        {
            // Set X and Y
            curRect.w = (TILE_SIZE/2) / MINIMAP_DIVIDER;
            curRect.h = (TILE_SIZE/2) / MINIMAP_DIVIDER;
            curRect.x = (path.nodes[i]->gridPos.x) * TILE_SIZE / MINIMAP_DIVIDER +1;
            curRect.y = (path.nodes[i]->gridPos.y) * TILE_SIZE / MINIMAP_DIVIDER +1;


            // If it is an empty space
            R_BlitColorIntoScreen(SDL_MapRGB(win_surface->format, 255, 0, 255), &curRect);
            SDL_UpdateWindowSurface(application.win);
            SDL_Delay(20);
        }

        SDL_Delay(4000);

        // Return it
        return path;
    }
    else
    {
        // Path is invalid
        path.isValid = false;
        return path;
    }
}

void I_InsertNode(int level, pathnode_t* node, int gridx, int gridy, int f, int g, int h, pathnode_t* parent, bool debug, sprite_t* entity)
{
    if(visited[gridy][gridx] || G_CheckCollisionMap(level, gridy, gridx) > 0 || G_CheckDynamicSpriteMap(level, gridy, gridx) && G_GetFromDynamicSpriteMap(level, gridy, gridx) != entity)
        return;

    node->gridPos.x = gridx;
    node->gridPos.y = gridy;
    node->f = f;
    node->g = g;
    node->h = h;
    node->parent = parent;

    visited[gridy][gridx] = true;

    frontierLength++;

    if(debug)
    {
        SDL_Rect curRect;
        // Set X and Y
        curRect.w = TILE_SIZE / MINIMAP_DIVIDER;
        curRect.h = TILE_SIZE / MINIMAP_DIVIDER;
        curRect.x = gridx * TILE_SIZE / MINIMAP_DIVIDER;
        curRect.y = gridy * TILE_SIZE / MINIMAP_DIVIDER;

        // If it is an empty space
        R_BlitColorIntoScreen(SDL_MapRGB(win_surface->format, 0, 255, 0), &curRect);
        SDL_Delay(5);
        SDL_UpdateWindowSurface(application.win);
    }
}

void I_AddAdiacentNodes(int level, int oGridX, int oGridY, pathnode_t* parent, bool debug, sprite_t* entity)
{
    // Top
    if(oGridY-1 >= 0 && !visited[oGridY-1][oGridX])
        I_InsertNode(level, &frontier[frontierLength], oGridX, oGridY-1, 0,0,0, parent, debug, entity);

    // Bottom
    if(oGridY+1 < MAP_HEIGHT && !visited[oGridY+1][oGridX])
        I_InsertNode(level, &frontier[frontierLength], oGridX, oGridY+1, 0,0,0, parent, debug, entity);

    // Left
    if(oGridX-1 >= 0 && !visited[oGridY][oGridX-1])
        I_InsertNode(level, &frontier[frontierLength], oGridX-1, oGridY, 0,0,0, parent, debug, entity);

    // Right
    if(oGridX+1 < MAP_WIDTH && !visited[oGridY][oGridX+1])
        I_InsertNode(level, &frontier[frontierLength], oGridX+1, oGridY, 0,0,0, parent, debug, entity);


    // Allow Diagonal movement and prevent corner cutting
    // Top Left
    if(oGridY-1 >= 0 && oGridX-1 >= 0 && !visited[oGridY-1][oGridX-1])
        if(G_CheckCollisionMap(level, oGridY-1, oGridX-1+1) == 0 && // check right
           G_CheckCollisionMap(level, oGridY-1+1, oGridX-1) == 0)   // check under
            I_InsertNode(level, &frontier[frontierLength], oGridX-1, oGridY-1, 0,0,0, parent, debug, entity);

    // Top Right
    if(oGridY-1 >= 0 && oGridX+1 < MAP_WIDTH && !visited[oGridY-1][oGridX+1])
        if(G_CheckCollisionMap(level, oGridY-1+1, oGridX+1) == 0 && // check under
           G_CheckCollisionMap(level, oGridY-1, oGridX+1-1) == 0)   // left
            I_InsertNode(level, &frontier[frontierLength], oGridX+1, oGridY-1, 0,0,0, parent, debug, entity);

    // Bottom Left
    if(oGridY+1 < MAP_HEIGHT && oGridX-1 >= 0 && !visited[oGridY+1][oGridX-1])
        if(G_CheckCollisionMap(level, oGridY+1, oGridX-1+1) == 0 && // check right
           G_CheckCollisionMap(level, oGridY+1-1, oGridX-1) == 0)   // check up
            I_InsertNode(level, &frontier[frontierLength], oGridX-1, oGridY+1, 0,0,0, parent, debug, entity);

    // Bottom Right
    if(oGridY+1 < MAP_HEIGHT && oGridX+1 < MAP_WIDTH && !visited[oGridY+1][oGridX+1])
        if(G_CheckCollisionMap(level, oGridY+1-1, oGridX+1) == 0 && // check under
           G_CheckCollisionMap(level, oGridY+1, oGridX+1-1) == 0)   // check left
            I_InsertNode(level, &frontier[frontierLength], oGridX+1, oGridY+1, 0,0,0, parent, debug, entity);
}

bool G_CheckDynamicSpriteMap(int level, int y, int x)
{
    if(x >= 0 && y >= 0 && x < MAP_WIDTH && y < MAP_HEIGHT)
    {
        switch(level)
        {
            case 0:
                return (currentMap.dynamicSpritesLevel0[y][x]) != NULL ? currentMap.dynamicSpritesLevel0[y][x] : NULL;

            case 1:
                return (currentMap.dynamicSpritesLevel1[y][x]) != NULL ? currentMap.dynamicSpritesLevel1[y][x] : NULL;

            case 2:
                return (currentMap.dynamicSpritesLevel2[y][x]) != NULL ? currentMap.dynamicSpritesLevel2[y][x] : NULL;

            default:
                //printf("WARNING! Level get was out of max/min level size\n");
                return false;
        }
    }
    else
    {
        return false;
    }
}

sprite_t* G_GetFromDynamicSpriteMap(int level, int y, int x)
{
    if(x >= 0 && y >= 0 && x < MAP_WIDTH && y < MAP_HEIGHT)
    {
        switch(level)
        {
            case 0:
                return (currentMap.dynamicSpritesLevel0[y][x]);

            case 1:
                return (currentMap.dynamicSpritesLevel1[y][x]);

            case 2:
                return (currentMap.dynamicSpritesLevel2[y][x]);

            default:
                //printf("WARNING! Level get was out of max/min level size\n");
                return NULL;
        }
    }
    else
    {
        return NULL;
    }
}
