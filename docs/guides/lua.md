---
layout: page
title: Lua Functions
permalink: /guides/lua/
---

* TOC
{:toc}

### Input functions

All the functions in this section are only valid inside `onInput()` callback, 
and do nothing when the movie is in playback mode. Here are the indices of the
different controls:

| Index | Mouse button | Controller button | Controller axis |
| ----- | ------------ | ----------------- | --------------- |
| 0     | Left         | A                 | Left stick X    |
| 1     | Middle       | B                 | Left stick Y    |
| 2     | Right        | X                 | Right stick X   |
| 3     | X1           | Y                 | Right stick Y   |
| 4     | X2           | Select / Back     | Left trigger    |
| 5     |              | Guide             | Right trigger   |
| 6     |              | Start             |                 |
| 7     |              | Left stick        |                 |
| 8     |              | Right stick       |                 |
| 9     |              | Left shoulder     |                 |
| 10    |              | Right shoulder    |                 |
| 11    |              | Dpad up           |                 |
| 12    |              | Dpad down         |                 |
| 13    |              | Dpad left         |                 |
| 14    |              | Dpad right        |                 |

#### input.clear

    none input.clear()

Clear the current inputs to default values.

#### input.setKey

    none input.setKey(Number keysym, Number state)
    
Set the specified keysym to the state `released` (0) or `pressed` (anything else).

#### input.getKey

    Number input.getKey(Number keysym)

Returns the state of the specified keysym (0: `released`, 1: `pressed`).

#### input.setMouseCoords

    none input.setMouseCoords(Number x, Number y, Number mode)
    
Set the mouse pointer coordinates and mode (0: `absolute`, 1: `relative`).

#### input.getMouseCoords

    (Number x, Number y, Number mode) input.getMouseCoords()
    
Returns the mouse pointer coordinates and mode (0: `absolute`, 1: `relative`).

#### input.setMouseButtons

    none input.setMouseButtons(Number button, Number state)
    
Set the mouse specified button to state `released` (0) or `pressed` (anything else).

#### input.getMouseButtons

    Number input.getMouseButtons(Number button)
    
Returns the mouse state of specified button (0: `released`, 1: `pressed`).

#### input.setControllerButton

    none input.setControllerButton(Number controller, Number button, Number state)
    
Set the button `button` of controller `controller` to state `released` (0) or `pressed` (anything else).

#### input.getControllerButton

    Number input.getControllerButton(Number controller, Number button)
    
Returns the state of specified controller button (0: `released`, 1: `pressed`).

#### input.setControllerAxis

    none input.setControllerAxis(Number controller, Number button, Number value)
    
Set the axis `axis` of controller `controller` to specified value (accepted range -32768 to 32767).

#### input.getControllerAxis

    Number input.getControllerAxis(Number controller, Number button)
    
Returns the value of specified controller axis.

### Movie functions

#### movie.currentFrame

    Number movie.currentFrame()

Returns the current frame number.

#### movie.frameCount

    Number movie.frameCount()

Returns the number of frames in the movie, or -1 if no movie is loaded

#### movie.status

    Number movie.status()

Returns the recording status of the movie:
* 0: no movie is loaded
* 1: movie is in recording mode
* 2: movie is in playback mode

#### movie.time

    (Number seconds, Number nseconds) movie.time()

Returns the current system time. `seconds` is in whole seconds (since 19700101T000000Z)
and `nseconds` is the number of nanoseconds.

#### movie.rerecords

    Number movie.rerecords()

Returns the current rerecord count of the movie, or -1 if no movie is loaded

### Callbacks

These functions, if defined in the lua script, are called at specific moments
during the game execution. 

#### onStartup

Called when the game has started, before the end of the first frame.

#### onInput

Called when input for the current frame is decided. Change the movie input if
in recording mode. Not called when movie is in playback mode.

#### onFrame

Called after frame has completed.
