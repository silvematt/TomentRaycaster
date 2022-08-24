# TomentRaycaster
A Software Rendering Raycaster Engine written in C and SDL2.

![alt text](https://i.imgur.com/c2uSgeR.png)

I've written this raycaster following Permadi's paper and Wolfenstein 3D code (and Black Book), as a pilgrimage to my God John Carmack.

I've loved the time I've spent on it, it's a work in progress with other features on schedule :)

Features:
- Rendering 640x480
- Player Input (Walk forward/backwards, rotate, strafe, interact)
- Textured Walls, Floors and Ceilings
- Thin (offsetted) Walls
- Sliding Doors
- Transparent (see through) doors and thin walls
- Variable Ceiling Height for each map
- Variable Ceiling Height for each tile of the map, allowing the creation of smaller rooms and sectors in the same map.
- Sprites Rendering & Physics (Sprites can be solid or not)
- Support for Animated Sprites
- Shading and Map Lighting
- Load map from file
- Assets Manager
- Packed data in custom files (.archt) format [GENERATED WITH: [TomentARCH](https://github.com/silvematt/TomentARCH)]
- Minimap
- Fixed Time Step
