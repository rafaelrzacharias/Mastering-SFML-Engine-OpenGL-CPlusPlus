# Mastering SFML Engine OpenGL C++

This was my third project.

I decided to learn OpenGL and go deeper into C++ for games.

The project used SFML (C++ library) and OpenGL(GLEW). All dependencies are included

Download the complete project with solution and placement art, build and enjoy!


NOTES:

- I tried to wrap every dependency needed into the Dependency folder.
In case the compiler gets angry, just download MVC++ Redistributable and install.
- I did a diagram showing all classes, their relationships, and their engine sub-systems color-coded,
so one person can get the full grasp of what is under the hood.
- I started worrying about memory management and overall engine performance.
- I build the entire game engine in such a way that most DATA is now separated from CODE,
I did this because I learned that another programer, myself, or a game designer would like to tweak
things without the need to wait several minutes each time. So, most classes and objects are are "fed"
with attributes and values from some TXT configuration file.
- Everything was done from scratch, built on top of SFML multimidia layer, all it does is interface with video and audio drivers.
- For this project, I learned a lot about game engine architecture and high level decision regarding efficiency, code base organization and extensibility.
I read a lot about programming patterns and managing large code bases efficiently, data encapsulation, Object Oriented and Composition Principles.
The result was an architecture that I built where the game uses Object Oriented approach whenever needed and for the overall structure the engine uses
systems (renderer, sound, state, movement, etc..) and components to interface with the systems. Game objects are now a collection of components, driven by DATA.
- Audio engine was coded, I learned how to create sound banks and wave banks for both sound effects (cues) and music (soundtracks),
and these can be loaded from file in the beginning of each level, be positioned in a 3D space in relation to the listner. Audio can be played, stopped,
paused, and now changed volume! My next goal is to learn about audio effects: I want to do things to audio such as echo, playbackwards, reverb, pitch, etc..
- Graphics engine was coded to interface with OpenGL and I learned a lot about graphics programming and also shaders programming (GLSL)
and fragment and vertex shaders. As an example, the game has a simple night/day cycle.
- Spritesheet was redone from scratch and now has many more features, such as loading frame data from a spritesheet in rows and columns,
specify timming between frames, loop around animation, stop animation, etc.
- I finally researched online and learned how to create GUI elements from scratch. I coded buttons, slidebars, checkboxes, and now
I can also specify GUI element reactions such as mouse hover, closed, clicked, minimized, maximized, window dragged (that's cool!).
- With my newly created GUI features I couldn't hold myself any longer and implemented shortly thereafter a level editor (even more cool!)
and now, on top of being able to specify data in text files, any programmer or designer can quickly sketch a map area, given all the art assets are provided.
- My most in-depth particle system was created. It focuses on efficiency, reusability of particles and quick generation of particles.
I can randomize and fill the screen with smoke, dancing flames of any shape, rain, snow, blood splatters and any effect I can imagine, all from a text file.
- My older collision class was completely redone to offer more features. Collision mode can now be AABB (alligned box), SAT (rotated box), circle or pixel perfect.
- Game engine States class is also upgraded, now it is easy to switch between game modes such as intro, title screen, in-game level, credits, map editor, etc..
- For this example, the game is a tile-based, grid-based, top view 2D game, but really any 2D game can be made with this engine.
