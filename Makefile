add:
gcc -fcommon -Isrc/include -Lsrc/lib -o bin/Toment.exe src/Engine/A_Application.c src/Engine/I_InputHandling.c src/Engine/M_Map.c src/Engine/R_Rendering.c src/Engine/G_Player.c src/Engine/G_Game.c src/main.c src/Engine/D_AssetsManager.c src/Engine/G_Physics.c src/Engine/U_Timer.c -lmingw32 -lSDL2main -lSDL2