#include "M_Map.h"
#include "G_Game.h"

map_t currentMap;

static void I_LoadMapFromFile(int map[MAP_HEIGHT][MAP_WIDTH], FILE* fp);
static void I_LoadIntFromFile(FILE* fp, int* toLoad);
static void I_LoadFloatFromFile(FILE* fp, float* toLoad);
static void I_ReadStringFromFile(FILE* fp, char toWrite[MAX_STRLEN]);
static void I_LoadDynamicSprite(sprite_t* cur, int spriteID, int x, int y);

// -------------------------------
// Loads the map from the file named mapID
// -------------------------------
void M_LoadMapAsCurrent(char* mapID)
{
      FILE* fp;
      char filepath[MAX_FILEPATH_L] = "Data/maps/";

      // Find path
      strcat(filepath, mapID);
      strcat(filepath, ".tmap");

      printf("Loading map... %s\n", filepath);

      // Open file
      fp = fopen(filepath, "r");

      if(fp == NULL)
      {
            printf("Map %s is not present in the Data folder. Aborting. \n");
            return;
      }

      // Load Map
      char curLine[MAX_STRL_R];   // Current line we're reading
      char* str;                  // Used to strchr
      int indx;                   // Index of the =
      int i;                      // Index for writing in new string

      // --------------------
      // Read ID
      // --------------------
      fgets(curLine, MAX_STRL_R, fp);

      // Find index for reading
      str = strchr(curLine, '=');
      indx = (int)(str - curLine) + 1;

      // Init index for writing
      i = 0;
      
      // Write
      while(curLine[indx] != ';' && curLine[indx] != '\n' && curLine[indx] != EOF)
      {
            currentMap.id[i] = curLine[indx];
            i++;
            indx++;
      }
      currentMap.id[i] = '\0';

      // --------------------
      // Read MapName
      // --------------------
      fgets(curLine, MAX_STRL_R, fp);

      // Find index for reading
      str = strchr(curLine, '=');
      indx = (int)(str - curLine) + 1;

      // Init index for writing
      i = 0;

      // Write
      while(curLine[indx] != ';' && curLine[indx] != '\n' && curLine[indx] != EOF)
      {
            currentMap.name[i] = curLine[indx];
            i++;
            indx++;
      }
      currentMap.name[i] = '\0';


      I_LoadIntFromFile(fp, &currentMap.playerStartingLevel);

      // Load Starting player pos
      I_LoadIntFromFile(fp, &currentMap.playerStartingGridX);
      I_LoadIntFromFile(fp, &currentMap.playerStartingGridY);
      I_LoadFloatFromFile(fp, &currentMap.playerStartingRot);

      // Load Wall Map
      I_LoadMapFromFile(currentMap.level0, fp);
      I_LoadMapFromFile(currentMap.level1, fp);
      I_LoadMapFromFile(currentMap.level2, fp);

      // Load Floor Map
      I_LoadMapFromFile(currentMap.floorMap, fp);

      // Load Ceiling Map
      I_LoadMapFromFile(currentMap.ceilingMap, fp);
      
      // Load Sprites Map
      I_LoadMapFromFile(currentMap.spritesMapLevel0, fp);
      I_LoadMapFromFile(currentMap.spritesMapLevel1, fp);
      I_LoadMapFromFile(currentMap.spritesMapLevel2, fp);

      // Read Wall Lighting
      I_LoadFloatFromFile(fp, &currentMap.wallLight);

      // Read Ceiling Lighting
      I_LoadFloatFromFile(fp, &currentMap.floorLight);

      // Read SkyID
      I_LoadIntFromFile(fp, &currentMap.skyID);
      
      fclose(fp);

      // Load the TMap
      M_LoadObjectTMap();

      // Load the Collision Map
      M_LoadCollisionMaps();

      printf("Map loaded successfully! ID: %s\n", currentMap.id);
}

// -------------------------------
// Loads the object map
// -------------------------------
void M_LoadObjectTMap(void)
{
      for(int i = 0; i < allDynamicSpritesLength; i++)
            free(allDynamicSprites[i]);

      for(int y = 0; y < MAP_HEIGHT; y++)
            for(int x = 0; x < MAP_WIDTH; x++)
                  {
                        if(currentMap.dynamicSprites[y][x] != NULL)
                              currentMap.dynamicSprites[y][x] = NULL;
                  }

      allDynamicSpritesLength = 0;
      
      for(int y = 0; y < MAP_HEIGHT; y++)
            for(int x = 0; x < MAP_WIDTH; x++)
                  {     
                        // LEVEL 0 
                        // Initialize
                        currentMap.objectTMapLevel0[y][x] = ObjT_Empty;

                        // Check if it's a wall
                        int wallID = currentMap.level0[y][x];
                        if(wallID > 0)
                        {
                              currentMap.objectTMapLevel0[y][x] = ObjT_Wall;

                              // Check if it is a door
                              if(U_GetBit(&tomentdatapack.walls[wallID]->flags, 2) == 1)
                              {
                                    currentMap.objectTMapLevel0[y][x] = ObjT_Door;
                              }

                              if (U_GetBit(&tomentdatapack.walls[wallID]->flags, 3) == 1)
                              {
                                    currentMap.objectTMapLevel0[y][x] = ObjT_Trigger;
                              }
                        }

                        // Check if it's a sprite (overrides doors, but spirtes should never be placed on top of walls)
                        int spriteID = currentMap.spritesMapLevel0[y][x];
                        if(spriteID > 0)
                        {
                              currentMap.objectTMapLevel0[y][x] = ObjT_Sprite;

                              // Check if this is a dynamic sprite
                              if(U_GetBit(&tomentdatapack.sprites[spriteID]->flags, 2) == 1)
                              {
                                    // Construct the dynamic sprite in base of its id
                                    currentMap.dynamicSprites[y][x] = (sprite_t*)malloc(sizeof(sprite_t));
                                    sprite_t* cur = currentMap.dynamicSprites[y][x];

                                    I_LoadDynamicSprite(cur, spriteID, x, y);

                                    // The rest is calculated at runtime
                              }
                        }


                        // LEVEL 1
                        // Initialize
                        currentMap.objectTMapLevel1[y][x] = ObjT_Empty;

                        // Check if it's a wall
                        wallID = currentMap.level1[y][x];
                        if(wallID > 0)
                        {
                              currentMap.objectTMapLevel1[y][x] = ObjT_Wall;

                              // Check if it is a door
                              if(U_GetBit(&tomentdatapack.walls[wallID]->flags, 2) == 1)
                              {
                                    currentMap.objectTMapLevel1[y][x] = ObjT_Door;
                              }
                              if (U_GetBit(&tomentdatapack.walls[wallID]->flags, 3) == 1)
                              {
                                    currentMap.objectTMapLevel1[y][x] = ObjT_Trigger;
                              }
                        }

                        // Check if it's a sprite (overrides doors, but spirtes should never be placed on top of walls)
                        spriteID = currentMap.spritesMapLevel1[y][x];
                        if(spriteID > 0)
                        {
                              currentMap.objectTMapLevel1[y][x] = ObjT_Sprite;

                              // Check if this is a dynamic sprite
                              if(U_GetBit(&tomentdatapack.sprites[spriteID]->flags, 2) == 1)
                              {
                                    currentMap.dynamicSprites[y][x] = (sprite_t*)malloc(sizeof(sprite_t));
                                    sprite_t* cur = currentMap.dynamicSprites[y][x];
                                    
                                    I_LoadDynamicSprite(cur, spriteID, x, y);

                                    // The rest is calculated at runtime
                              }
                        }


                        // LEVEL 2
                        // Initialize
                        currentMap.objectTMapLevel2[y][x] = ObjT_Empty;

                        // Check if it's a wall
                        wallID = currentMap.level2[y][x];
                        if(wallID > 0)
                        {
                              currentMap.objectTMapLevel2[y][x] = ObjT_Wall;

                              // Check if it is a door
                              if(U_GetBit(&tomentdatapack.walls[wallID]->flags, 2) == 1)
                              {
                                    currentMap.objectTMapLevel2[y][x] = ObjT_Door;
                              }
                              if (U_GetBit(&tomentdatapack.walls[wallID]->flags, 3) == 1)
                              {
                                    currentMap.objectTMapLevel2[y][x] = ObjT_Trigger;
                              }
                        }

                        // Check if it's a sprite (overrides doors, but spirtes should never be placed on top of walls)
                        spriteID = currentMap.spritesMapLevel2[y][x];
                        if(spriteID > 0)
                        {
                              currentMap.objectTMapLevel2[y][x] = ObjT_Sprite;

                              // Check if this is a dynamic sprite
                              if(U_GetBit(&tomentdatapack.sprites[spriteID]->flags, 2) == 1)
                              {
                                    currentMap.dynamicSprites[y][x] = (sprite_t*)malloc(sizeof(sprite_t));
                                    sprite_t* cur = currentMap.dynamicSprites[y][x];
                                    
                                    I_LoadDynamicSprite(cur, spriteID, x, y);

                                    // The rest is calculated at runtime
                              }
                        }
                  }
}

// -------------------------------
// Loads the collision map
// -------------------------------
void M_LoadCollisionMaps(void)
{
      // Load Level 0
      for(int y = 0; y < MAP_HEIGHT; y++)
            for(int x = 0; x < MAP_WIDTH; x++)
                  {
                        // LEVEL 0
                        // Initialize
                        currentMap.collisionMapLevel0[y][x] = 0;

                        // Check if it's a wall
                        int wallID = currentMap.level0[y][x];
                        if(wallID > 0)
                              currentMap.collisionMapLevel0[y][x] = 1;

                        // Check if it's a sprite (overrides doors, but spirtes should never be placed on top of walls)
                        int spriteID = currentMap.spritesMapLevel0[y][x];
                        if(spriteID > 0 && U_GetBit(&tomentdatapack.sprites[spriteID]->flags, 0) == 1 &&
                              U_GetBit(&tomentdatapack.sprites[spriteID]->flags, 2) == 0)
                              currentMap.collisionMapLevel0[y][x] = 1;


                        // LEVEL 1
                        // Initialize
                        currentMap.collisionMapLevel1[y][x] = 0;

                        // Check if it's a wall
                        wallID = currentMap.level1[y][x];
                        if(wallID > 0)
                              currentMap.collisionMapLevel1[y][x] = 1;

                        // Check if it's a sprite (overrides doors, but spirtes should never be placed on top of walls)
                        spriteID = currentMap.spritesMapLevel1[y][x];
                        if(spriteID > 0 && U_GetBit(&tomentdatapack.sprites[spriteID]->flags, 0) == 1 &&
                              U_GetBit(&tomentdatapack.sprites[spriteID]->flags, 2) == 0)
                              currentMap.collisionMapLevel1[y][x] = 1;

                        // LEVEL 2
                        // Initialize
                        currentMap.collisionMapLevel2[y][x] = 0;

                        // Check if it's a wall
                        wallID = currentMap.level2[y][x];
                        if(wallID > 0)
                              currentMap.collisionMapLevel2[y][x] = 1;

                        // Check if it's a sprite (overrides doors, but spirtes should never be placed on top of walls)
                        spriteID = currentMap.spritesMapLevel2[y][x];
                        if(spriteID > 0 && U_GetBit(&tomentdatapack.sprites[spriteID]->flags, 0) == 1 &&
                              U_GetBit(&tomentdatapack.sprites[spriteID]->flags, 2) == 0)
                              currentMap.collisionMapLevel2[y][x] = 1;
                  }
}

static void I_LoadMapFromFile(int map[MAP_HEIGHT][MAP_WIDTH], FILE* fp)
{
      // Load Map
      char curLine[MAX_STRL_R];   // Current line we're reading
      char* str;                  // Used to strchr
      int indx;                   // Index of the =
      int i;                      // Index for writing in new string

      fgets(curLine, MAX_STRLEN, fp); // Layout =
      fgets(curLine, MAX_STRLEN, fp); // [ start of map
      fgets(curLine, MAX_STRLEN, fp); // First Row

      // Find the first row
      str = strchr(curLine, '{');
      indx = (int)(str - curLine) + 1;

      bool mapDone = false;
      int column = 0;
      int row = 0;
      bool rowEnded = false;

      while(!mapDone)
      {
            // Read columns
            while(curLine[indx] != '}')
            {
                  map[column][row] = curLine[indx] - '0'; // Set int value
                  indx++;  

                  // If next is comma, continue and get next number
                  if(curLine[indx] == ',')
                  {
                        indx++;
                        row++;
                  }

                  //printf("%c!\n", curLine[indx]);
            }

            // Row end, check if there's a next row or if it is finished
            if(curLine[indx + 1] == ',')
            {
                  // There is a next column
                  column++;
                  indx = 1; // Move at the start of the next column
                  row = 0;
                  fgets(curLine, MAX_STRLEN, fp); // Get next line
                  continue;
            }
            else if(curLine[indx + 1] == ']')
            {
                  // Map has finished loading
                  mapDone = true;
                  break;
            }
      }
}

static void I_LoadIntFromFile(FILE* fp, int* toLoad)
{
      // Load Map
      char curLine[MAX_STRL_R];   // Current line we're reading
      char* str;                  // Used to strchr
      int indx;                   // Index of the =
      int i;                      // Index for writing in new string

      // --------------------
      // Read SkyID
      // --------------------
      fgets(curLine, MAX_STRLEN, fp); // Layout =

      // Find index for reading
      str = strchr(curLine, '=');
      indx = (int)(str - curLine) + 1;

      // Init index for writing
      i = 0;
      
      char tempStr[256];
      // Write
      while(curLine[indx] != ';' && curLine[indx] != '\n' && curLine[indx] != EOF)
      {
            tempStr[i] = curLine[indx];
            i++;
            indx++;
      }
      tempStr[i] = '\0';

      // Convert to float
      *toLoad = atoi(tempStr);
}

static void I_LoadFloatFromFile(FILE* fp, float* toLoad)
{
      // Load Map
      char curLine[MAX_STRL_R];   // Current line we're reading
      char* str;                  // Used to strchr
      int indx;                   // Index of the =
      int i;                      // Index for writing in new string

      // --------------------
      // Read SkyID
      // --------------------
      fgets(curLine, MAX_STRLEN, fp); // Layout =

      // Find index for reading
      str = strchr(curLine, '=');
      indx = (int)(str - curLine) + 1;

      // Init index for writing
      i = 0;
      
      char tempStr[256];
      // Write
      while(curLine[indx] != ';' && curLine[indx] != '\n' && curLine[indx] != EOF)
      {
            tempStr[i] = curLine[indx];
            i++;
            indx++;
      }
      tempStr[i] = '\0';

      // Convert to float
      *toLoad = atof(tempStr);
}

static void I_ReadStringFromFile(FILE* fp, char toWrite[MAX_STRLEN])
{
      // Load Map
      char curLine[MAX_STRL_R];   // Current line we're reading
      char* str;                  // Used to strchr
      int indx;                   // Index of the =
      int i;                      // Index for writing in new string

      fgets(curLine, MAX_STRL_R, fp);

      printf("%s\n", curLine);

      // Find index for reading
      str = strchr(curLine, '=');
      indx = (int)(str - curLine) + 1;

      // Init index for writing
      i = 0;

      // Write
      while(curLine[indx] != ';' && curLine[indx] != '\n' && curLine[indx] != EOF)
      {
            toWrite[i] = curLine[indx];
            i++;
            indx++;
      }
      toWrite[i] = '\0';
}

static void I_LoadDynamicSprite(sprite_t* cur, int spriteID, int x, int y)
{
      cur->active = true;
      cur->level = 0;
      cur->speed = 2.0f;

      cur->gridPos.x = x;
      cur->gridPos.y = y;

      // Get World Pos
      cur->pos.x = x * TILE_SIZE;
      cur->pos.y = y * TILE_SIZE;

      cur->collisionCircle.pos.x = cur->pos.x;
      cur->collisionCircle.pos.y = cur->pos.y;
      cur->collisionCircle.r = 32;

      // Get ID
      cur->spriteID = spriteID;
      cur->sheetLength = tomentdatapack.spritesSheetsLenghtTable[spriteID];

      // Add it to the dynamic sprite list
      allDynamicSprites[allDynamicSpritesLength] = cur;
      allDynamicSpritesLength++;
}