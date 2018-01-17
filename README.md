# Mastering SFML Engine OpenGL C++

This was my third project.

I decided to learn OpenGL and go deeper into C++ for games.

The project used SFML (C++ library) and OpenGL (GLEW). Everything is in the Dependencies folder.

In case the compiler gets angry, just download MVC++ Redistributable and install.

Download the complete project with solution and placement art, build and enjoy!


NOTES:

- I did a diagram showing all classes, their relationships, and their engine sub-systems color-coded,
so one person can get the full grasp of what is under the hood.
- I started worrying about memory management and overall engine performance.
- The entire game engine is build so that DATA is now separated from CODE. This makes it easy to tweak variables without recompiling.
- Most object initialization is now performed with values read from some .txt file.
- I learned a lot about game engine architecture and game programming patterns in order to manage large code bases efficiently.
- The overall structure of the engine uses systems (renderer, sound, state, movement) and components to interface with said systems. Game objects are now a collection of components, driven by data.
- I learned how to create sound banks and wave banks for both sound effects (cues) and music (soundtracks). These can be loaded from file in the beginning of each level, and positional audio. Audio can be played, stopped, paused, and changed volume.
- I learned a lot about graphics programming, shaders programming (GLSL), fragment and vertex shaders. As an example, the game has a simple night/day cycle.
- Spritesheet was redone from scratch and now has many more features, such as loading frame data from a spritesheet in rows and columns,
specify timming between frames, loop around animation, stop animation.
- GUI elements were coded to offer buttons, slidebars, checkboxes and UI reactions such as mouse hover, closed, clicked, minimized, maximized, window dragged. To showcase the GUI system, a level editor was implemented.
- An in-depth particle system was created. It focuses on efficiency, reusability of particles and quick generation of particles. Most particle effects (smoke, rain, flames, snow, blood...) can be created now from a .txt file.
- Collision mode can now be AABB (axis-alligned bounding box), SAT (rotated box), circle or pixel perfect.
- GameState class makes it possible to create and switch between different game modes such as intro, title screen, level, credits, map editor.
- The example game is a top down 2D rpg, but any 2D game can be made with this engine with little modifications.
