# Game specific behaviors

By default, games are using SDL.

### Super Meat Boy

- Use OpenGL for rendering
- Use OpenAL for music
- Use pthread for thread creation
- Use Vsync

### Volgarr

- Waiting screen at the beginning, expecting increasing SDL GetTicks()
- Does a few SDL sleep of 16 ms, but then rely on Vsync

