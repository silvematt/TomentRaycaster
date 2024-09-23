#OBJS specifies which files to compile as part of the project
OBJS = src/Engine/A_Application.c src/Engine/I_InputHandling.c src/Engine/M_Map.c src/Engine/R_Rendering.c src/Engine/G_Player.c src/Engine/G_Game.c src/main.c src/Engine/D_AssetsManager.c src/Engine/P_Physics.c src/Engine/U_Timer.c src/Engine/U_Utilities.c src/Engine/T_TextRendering.c src/Engine/G_MainMenu.c src/Engine/D_ObjectsCallbacks.c src/Engine/G_Pathfinding.c src/Engine/G_AI.c src/Engine/G_AIBehaviour.c src/toment.o

#CC specifies which compiler we're using
CC = gcc

#INCLUDE_PATHS specifies the additional include paths we'll need
INCLUDE_PATHS = -IC:\Lib\SDL2-2.30.1\include

#LIBRARY_PATHS specifies the additional library paths we'll need
LIBRARY_PATHS = -LC:\Lib\SDL2-2.30.1\lib\x86

#COMPILER_FLAGS specifies the additional compilation options we're using
COMPILER_FLAGS = -O2 -Wl,--subsystem,windows -mwindows

#LINKER_FLAGS specifies the libraries we're linking against
LINKER_FLAGS = -lmingw32 -lSDL2main -lSDL2 -fopenmp

#OBJ_NAME specifies the name of our exectuable
OBJ_NAME = bin/Toment

#This is the target that compiles our executable
all : $(OBJS)
	$(CC) $(OBJS) $(INCLUDE_PATHS) $(LIBRARY_PATHS) $(COMPILER_FLAGS) $(LINKER_FLAGS) -o $(OBJ_NAME)