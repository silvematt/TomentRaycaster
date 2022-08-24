#include "M_Map.h"

map_t currentMap;

// -------------------------------
// Loads the map from the file named mapID
// -------------------------------
void M_LoadMapAsCurrent(char* mapID)
{
      FILE* fp;
      char filepath[MAX_FILEPATH_L] = "Data/";

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

      // --------------------
      // Read Walls Map Layout
      // --------------------
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
                  currentMap.wallMap[column][row] = curLine[indx] - '0'; // Set int value
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
                  fgets(curLine, MAX_STRLEN, fp); // Get next line
                  break;
            }
      }
      
      // --------------------
      // Read Floor Map Layout
      // --------------------
      fgets(curLine, MAX_STRLEN, fp); // Layout =
      fgets(curLine, MAX_STRLEN, fp); // [ start of map

      fgets(curLine, MAX_STRLEN, fp); // First Row

            // Find the first row
      str = strchr(curLine, '{');
      indx = (int)(str - curLine) + 1;

      mapDone = false;
      column = 0;
      row = 0;
      rowEnded = false;

      while(!mapDone)
      {
            // Read columns
            while(curLine[indx] != '}')
            {
                  currentMap.floorMap[column][row] = curLine[indx] - '0'; // Set int value
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
                  fgets(curLine, MAX_STRLEN, fp); // Get next line
                  break;
            }
      }

      // --------------------
      // Read Ceiling Map Layout
      // --------------------
      fgets(curLine, MAX_STRLEN, fp); // Layout =
      fgets(curLine, MAX_STRLEN, fp); // [ start of map

      fgets(curLine, MAX_STRLEN, fp); // First Row

      // Find the first row
      str = strchr(curLine, '{');
      indx = (int)(str - curLine) + 1;

      mapDone = false;
      column = 0;
      row = 0;
      rowEnded = false;

      while(!mapDone)
      {
            // Read columns
            while(curLine[indx] != '}')
            {
                  currentMap.ceilingMap[column][row] = curLine[indx] - '0'; // Set int value
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
                  fgets(curLine, MAX_STRLEN, fp); // Get next line
                  break;
            }
      }

      // --------------------
      // Read Sprites Map Layout
      // --------------------
      fgets(curLine, MAX_STRLEN, fp); // Layout =
      fgets(curLine, MAX_STRLEN, fp); // [ start of map

      fgets(curLine, MAX_STRLEN, fp); // First Row

      // Find the first row
      str = strchr(curLine, '{');
      indx = (int)(str - curLine) + 1;

      mapDone = false;
      column = 0;
      row = 0;
      rowEnded = false;

      while(!mapDone)
      {
            // Read columns
            while(curLine[indx] != '}')
            {
                  currentMap.spritesMap[column][row] = curLine[indx] - '0'; // Set int value
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
                  fgets(curLine, MAX_STRLEN, fp); // Get next line
                  break;
            }
      }

      // --------------------
      // Read Wall Lighting
      // --------------------
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
      currentMap.wallLight = atof(tempStr);

      // --------------------
      // Read Ceiling Lighting
      // --------------------
      fgets(curLine, MAX_STRLEN, fp); // Layout =

      // Find index for reading
      str = strchr(curLine, '=');
      indx = (int)(str - curLine) + 1;

      // Init index for writing
      i = 0;
      
      // Write
      while(curLine[indx] != ';' && curLine[indx] != '\n' && curLine[indx] != EOF)
      {
            tempStr[i] = curLine[indx];
            i++;
            indx++;
      }
      tempStr[i] = '\0';

      // Convert to float
      currentMap.floorLight = atof(tempStr);

      // --------------------
      // Read Ceiling Height Map Layout
      // --------------------
      fgets(curLine, MAX_STRLEN, fp); // Layout =
      fgets(curLine, MAX_STRLEN, fp); // [ start of map

      fgets(curLine, MAX_STRLEN, fp); // First Row

      // Find the first row
      str = strchr(curLine, '{');
      indx = (int)(str - curLine) + 1;

      mapDone = false;
      column = 0;
      row = 0;
      rowEnded = false;

      currentMap.maxCeilingHeight = 0;
      while(!mapDone)
      {
            // Read columns
            while(curLine[indx] != '}')
            {
                  currentMap.ceilingHeightMap[column][row] = curLine[indx] - '0'; // Set int value
                  if(currentMap.ceilingHeightMap[column][row] > currentMap.maxCeilingHeight)
                        currentMap.maxCeilingHeight = currentMap.ceilingHeightMap[column][row];
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
                  fgets(curLine, MAX_STRLEN, fp); // Get next line
                  break;
            }
      }
      
      // --------------------
      // Read Orientation Map Layout
      // --------------------
      fgets(curLine, MAX_STRLEN, fp); // Layout =
      fgets(curLine, MAX_STRLEN, fp); // [ start of map

      fgets(curLine, MAX_STRLEN, fp); // First Row

      // Find the first row
      str = strchr(curLine, '{');
      indx = (int)(str - curLine) + 1;

      mapDone = false;
      column = 0;
      row = 0;
      rowEnded = false;
      
      while(!mapDone)
      {
            // Read columns
            while(curLine[indx] != '}')
            {
                  currentMap.orientationMap[column][row] = curLine[indx] - '0'; // Set int value
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
                  fgets(curLine, MAX_STRLEN, fp); // Get next line
                  break;
            }
      }

      printf("Map loaded successfully!\n");
      fclose(fp);

      // Load the TMap
      M_LoadObjectTMap();

      // Load the Collision Map
      M_LoadCollisionMap();
}

// -------------------------------
// Loads the object map
// -------------------------------
void M_LoadObjectTMap(void)
{
      for(int y = 0; y < MAP_HEIGHT; y++)
            for(int x = 0; x < MAP_WIDTH; x++)
                  {
                        // Initialize
                        currentMap.objectTMap[y][x] = ObjT_Empty;

                        // Check if it's a wall
                        int wallID = currentMap.wallMap[y][x];
                        if(wallID > 0)
                        {
                              // Check if it is a door
                              if(U_GetBit(&tomentdatapack.walls[wallID]->flags, 2) == 1)
                              {
                                    currentMap.objectTMap[y][x] = ObjT_Door;
                              }
                              else
                                    currentMap.objectTMap[y][x] = ObjT_Wall;
                        }

                        // Check if it's a sprite (overrides doors, but spirtes should never be placed on top of walls)
                        int spriteID = currentMap.spritesMap[y][x];
                        if(spriteID > 0)
                              currentMap.objectTMap[y][x] = ObjT_Sprite;
                  }
}

// -------------------------------
// Loads the collision map
// -------------------------------
void M_LoadCollisionMap(void)
{
      for(int y = 0; y < MAP_HEIGHT; y++)
            for(int x = 0; x < MAP_WIDTH; x++)
                  {
                        // Initialize
                        currentMap.collisionMap[y][x] = 0;

                        // Check if it's a wall
                        int wallID = currentMap.wallMap[y][x];
                        if(wallID > 0)
                              currentMap.collisionMap[y][x] = 1;

                        // Check if it's a sprite (overrides doors, but spirtes should never be placed on top of walls)
                        int spriteID = currentMap.spritesMap[y][x];
                        if(spriteID > 0 && U_GetBit(&tomentdatapack.sprites[spriteID]->flags, 0) == 1)
                              currentMap.collisionMap[y][x] = 1;
                  }
}