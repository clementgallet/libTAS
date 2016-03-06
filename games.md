# Game specific behaviors

### Super Meat Boy

- Use SDL for inputs
- Use SDL/OpenGL for rendering
- Use OpenAL for music
- Use pthread for thread creation
- Use Vsync

### Volgarr

- Use SDL for inputs
- Use SDL/OpenGL for rendering
- Waiting screen at the beginning, expecting increasing SDL GetTicks()
- Does a few SDL sleep of 16 ms, but then rely on Vsync

### Shovel Knight

- Use SDL for threads
- Enable Xlib events on SDL (what is it looking for?)

### Limbo

- Loading screen with threading at the beginning
- Get access to the real time using SDL GetPerformanceCounter and some other means...

### FEZ

- Use a mono framework, see https://github.com/flibitijibibo/FNA-MGHistory/tree/fez-sdl2
- Use SDL library but we couldn't hook any SDL function...

### Towerfall

- Same as FEZ

### Braid

- Multiple version have been published with SDL 1.2 or SDL 2
- It uses PeepEvents to look for SDL Events

