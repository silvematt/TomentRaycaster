#include "M_Map.h"

bool mapInFrameBuffer = false;

map_t currentMap;

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
      printf("%f", currentMap.wallLight);

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
      printf("%f", currentMap.floorLight);

      printf("Map loaded successfully!\n");
      fclose(fp);
}