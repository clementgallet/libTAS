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
- Get access to the real time by some means...

