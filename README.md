# TomentRaycaster
A Software Rendering Raycaster Engine written in C and SDL2.

![alt text](https://i.imgur.com/0YlbTOA.png)

I've written this raycaster following Permadi's paper and Wolfenstein 3D code (and Black Book), as a pilgrimage to my God John Carmack.

I've loved the time I've spent on it, it's a work in progress with other features on schedule :)

Features:
- Rendering 640x480
- Player Input (Walk forward/backwards, fly up/down, rotate, strafe, interact)
- Textured Walls, Floors, Ceilings
- Thin (offsetted) Walls
- Sliding Doors
- Transparent (see through) doors and thin walls
- Multi Floor support (for player, doors, walls, sprites etc.)
- Vertical Motion
- Sprites Rendering & Collisions
- AI and basic BFS Pathfinding
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
