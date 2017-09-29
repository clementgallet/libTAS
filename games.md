# Game specific behaviors

### Super Meat Boy

- 32 and 64-bit
- Use SDL for inputs
- Use SDL/OpenGL for rendering
- Use OpenAL for music
- Use pthread for thread creation

### Volgarr

- 32-bit
- Use SDL for inputs
- Use SDL/OpenGL for rendering
- Waiting screen at the beginning, expecting increasing SDL GetTicks()
- Does a few SDL sleep of 16 ms, but then rely on Vsync

### Shovel Knight

- 32 and 64-bit
- Use SDL for threads
- Enable Xlib events on SDL (what is it looking for?)
- Use pulseaudio-simple for music

### Limbo

- 32-bit
- Loading screen with threading at the beginning
- Get access to the real time using SDL GetPerformanceCounter and some other means...

### FEZ

- 32 and 64-bit
- Use a mono framework, see https://github.com/flibitijibibo/FNA-MGHistory/tree/fez-sdl2

### Towerfall

- Same as FEZ

### Braid

- Multiple version have been published with SDL 1.2 or SDL 2
- It uses PeepEvents to look for SDL Events
- Use SDL Audio for music
- Warp pointer and use deltas to move the game pointer

### Faster Than Light

- Use SDL 1.2

### VVVVVV

- Use SDL 2
- Use software rendering
- Use delays for constant fps

### Teslagrad

- Unity

### Thomas Was Alone

- Unity

### Bastion

- Use SDL 1.2 only for joystick
- Mono branch for the game: https://github.com/SupergiantGames/MonoGame/tree/linux_port

### Hotline Miami

- Use openGL inside qt4
- Look at /dev/input/jsX then open /dev/input/eventX using /proc/bus/input/devices for joysticks
- Use XQueryPointer for mouse
- Keyboard not yet hooked

### SteamWorld Dig

- Use XEvents for keyboard and inputs
- Use /dev/input/eventX for joystick

