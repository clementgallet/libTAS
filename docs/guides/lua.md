---
layout: page
title: Lua Functions
permalink: /guides/lua/
---

* TOC
{:toc}

### Gui functions

Gui functions are only valid in callback `onPaint()`. **Beware**, option
`Video > OSD > Lua` needs to be checked to show any lua draw function.
In all gui functions, colors are coded in a single 32-bit unsigned integer as followed: `0xaarrggbb`.

#### gui.resolution

    (number width, number height) gui.resolution()

Returns the width and height of game window. 

#### gui.text

    none gui.text(Number x, Number y, String text, [Number color = 0xffffffff])
    
Draws text string `text` starting from `(x, y)`. The text will be colored using color `color`.

#### gui.pixel

    none gui.pixel(Number x, Number y, [Number color = 0xffffffff]) 

Draws pixel at (x,y) using color `color`.

#### gui.line

    none gui.line(Number x0, Number y0, Number x1, Number y1, [Number color = 0xffffffff])

Draws an antialiased line starting from `(x0,y0)` to `(x1,y1)`, and colored with color `color`.

#### gui.rectangle

    none gui.rectangle(Number x, Number y, Number w, Number h, [Number thickness = 1], [Number color = 0xffffffff], [Number filled = 0])

Draws rectangle of size `(w,h)` with top-left corner at `(x,y)`. Outline is of thickness `thickness` and is colored with color `color`. If `filled` is true, the rectangle is filled with the same color.

#### gui.ellipse

    none gui.ellipse(Number center_x, Number center_y, Number radius_x, Number radius_y, [Number color = 0xffffffff])

Draws an antialiased ellipse whose center is `(center_x,center_y)`, radius along x-axis is `radius_x` and radius along y-axis is `radius_y`, colored with color `color`.

### Input functions

All the functions in this section are only valid inside `onInput()` callback.
Here are the indices of the different controls:

| Index | Mouse button | Controller button | Controller axis | Flag         |
| ----- | ------------ | ----------------- | --------------- | ------------ |
| 0     | Left         | A                 | Left stick X    | Restart      |
| 1     | Middle       | B                 | Left stick Y    | Joy1 added   |
| 2     | Right        | X                 | Right stick X   | Joy2 added   |
| 3     | X1           | Y                 | Right stick Y   | Joy3 added   |
| 4     | X2           | Select / Back     | Left trigger    | Joy4 added   |
| 5     |              | Guide             | Right trigger   | Joy1 removed |
| 6     |              | Start             |                 | Joy2 removed |
| 7     |              | Left stick        |                 | Joy3 removed |
| 8     |              | Right stick       |                 | Joy4 removed |
| 9     |              | Left shoulder     |                 |              |
| 10    |              | Right shoulder    |                 |              |
| 11    |              | Dpad up           |                 |              |
| 12    |              | Dpad down         |                 |              |
| 13    |              | Dpad left         |                 |              |
| 14    |              | Dpad right        |                 |              |

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

#### input.setFlag

    none input.setFlag(Number flag, Number state)
    
Set the specified flag to the state `cleared` (0) or `set` (anything else).

#### input.getFlag

    Number input.getFlag(Number flag)

Returns the state of the specified flag (0: `cleared`, 1: `set`).

#### input.setFramerate

    none input.setFramerate(Number num, Number den)
    
Set the framerate numerator (`num`) and denominator (`den`).

#### input.getFramerate

    (Number num, Number den) input.getFramerate()
    
Returns the framerate numerator (`num`) and denominator (`den`).

#### input.setRealtime

    none input.setRealtime(Number sec, Number nsec)
    
Set the realtime in seconds (`sec`) and nanoseconds (`nsec`) (since 19700101T000000Z).

#### input.getRealtime

    (Number sec, Number nsec) input.getRealtime()
    
Returns the realtime seconds (`sec`) and nanoseconds (`nsec`) (since 19700101T000000Z).

### Memory functions

#### memory.readu8 / memory.readu16 / memory.readu32 / memory.readu64

    Number memory.readu8(Number address)
    Number memory.readu16(Number address)
    Number memory.readu32(Number address)
    Number memory.readu64(Number address)

Returns the unsigned value read from address `address` (any error returns 0).

#### memory.reads8 / memory.reads16 / memory.reads32 / memory.reads64

    Number memory.reads8(Number address)
    Number memory.reads16(Number address)
    Number memory.reads32(Number address)
    Number memory.reads64(Number address)

Returns the signed value read from address `address` (any error returns 0).

#### memory.readf / memory.readd

    Number memory.readf(Number address)
    Number memory.readd(Number address)

Returns the float/double value read from address `address` (any error returns 0).

#### memory.write8 / memory.write16 / memory.write32 / memory.write64

    None memory.write8(Number address, Number value)
    None memory.write16(Number address, Number value)
    None memory.write32(Number address, Number value)
    None memory.write64(Number address, Number value)

Writes the value `value` to address `address`.

#### memory.writef / memory.writed

    None memory.writef(Number address, Number value)
    None memory.writed(Number address, Number value)

Writes the value `value` to address `address`.

#### memory.baseAddress

    Number memory.baseAddress(String file)

Returns the base address of the file mapped in memory whose name is `file`. The
file can either be specified as absolute path, or the filename. Returns `0` if
the file could not be found.

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

Returns the elapsed time since the game startup. `seconds` is in whole seconds
and `nseconds` is the number of nanoseconds.

#### movie.rerecords

    Number movie.rerecords()

Returns the current rerecord count of the movie, or -1 if no movie is loaded

#### movie.isDraw

    Number movie.isDraw()

Returns 1 of the current frame is a draw frame, or 0 if not.

### Callbacks

#### callback.onStartup

    None callback.onStartup(Function f)

Registers `f` to be called when the game has started, before the end of the first frame.

#### callback.onInput

    None callback.onInput(Function f)

Registers `f` to be called when input for the current frame is decided. Change the movie input if
in recording mode. Not called when movie is in playback mode.

#### callback.onFrame

    None callback.onFrame(Function f)

Registers `f` to be called after frame has completed.

#### callback.onPaint

    None callback.onPaint(Function f)

Registers `f` to be called at the end of the frame before the screen rendering 
is performed. All Gui functions must be executed inside this callback. To 
include Lua draws in encodes, `Video > OSD > OSD on video encode` can be checked.

### Callbacks (obsolete)

The old callback method consists on defining functions with specific names above,
which will be called at specific moments during the game execution.

#### onStartup

Called when the game has started, before the end of the first frame.

#### onInput

Called when input for the current frame is decided. Change the movie input if
in recording mode. Not called when movie is in playback mode.

#### onFrame

Called after frame has completed.

#### onPaint

Called at the end of the frame before the screen rendering is performed. All Gui
functions must be executed inside this callback. To include Lua draws in encodes,
`Video > OSD > OSD on video encode` can be checked.
