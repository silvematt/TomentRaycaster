# TomentRaycaster
A Software Rendering Raycaster Engine written in C and SDL2.

![alt text](https://i.imgur.com/7oomyDP.png)

Video: https://www.youtube.com/watch?v=XFCinBirBVw

<b>Check out [TomentOnline](https://github.com/silvematt/TomentOnline)! An upgraded version of this engine for multiplayer!</b>

I've written this raycaster following Permadi's paper and Wolfenstein 3D code (and Black Book), as a pilgrimage to my God John Carmack.

Features:
- Rendering 640x480 (with scalable raycasting resolution) 
- Player Input (Walk forward/backwards, fly up/down, rotate, look up/down, strafe, interact)
- Textured Walls, Floors, Ceilings
- Thin (offsetted) Walls
- Sliding Doors
- Linear Fog Rendering
- Transparent (see through) doors and thin walls
- Multi Floor support (for player, doors, walls, sprites etc.)
- Vertical Motion
- Usable ladders to go up/down levels.
- Sprites Rendering & Collisions
- AI and basic BFS Pathfinding
- Boss AI and possibility to add custom behaviours for each AI (melee, casters, bosses)
- Player and AI combat
- Projectiles & Player's spells
- Pickups of weapons and consumables (potions) for the player
- Support for Animated Sprites
- Seamless Sky Rendering or Absolute Ceiling Rendering (or a mix of the two)
- Shading and Map Lighting
- Text Rendering of Bitmap Fonts
- Weapon and Spell switching
- Player HUD (Healthbar, Manabar, Equipped Weapon, Equipped Spell)
- Game Menus
- Load map from file
- Assets Manager
- Packed data in custom files (.archt) format [GENERATED WITH: [TomentARCH](https://github.com/silvematt/TomentARCH)]
- Minimap
- Fixed Time Step
