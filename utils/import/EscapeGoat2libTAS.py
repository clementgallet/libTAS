#!/usr/bin/env python
# This script converts Escape Goat 2 files
# (https://docs.google.com/spreadsheets/d/1i6WqCW9MefxQvWD8YkxFySavlF2jE6ULiEvQRhaJ3r0/edit#gid=0) to libTAS input file.
# Just run ./EscapeGoat2libTAS path/to/tasfile

import re
import sys
import os
import math

K_LEFT = 0
K_RIGHT = 1
K_UP = 2
K_DOWN = 3
K_JUMP = 4
K_DASH = 5
K_LIGHT = 6
K_HEAVY = 7
K_ESC = 8

input_file = open(sys.argv[1], 'r')
output_file = open(os.path.splitext(sys.argv[1])[0]+'.ltm' , 'w')

regex_input = re.compile(r'(\+?[\d]+)((,[0-9a-z]+)*)')

# prev_input_string = '000000000'
# prev_input = -1
# prev_input_frame = -1
frame = -1
held_keyboard_state = '||\n'

for line in input_file:
    match = regex_input.match(line)
    if match:

        # Process the frame count of the current input
        nbFrames = match.group(1)

        # First line is different, it is blank inputs but does not raise frame counter
        if frame == -1:
            nbFrames = int(nbFrames)
            frame = 0
        elif nbFrames[0] == '+':
            nbFrames = int(nbFrames[1:]) - 1 # -1 because we already wrote one frame on the previous iteration
            frame += nbFrames
        else:
            nbFrames = int(nbFrames) - frame
            frame += nbFrames

        # Write the previous inputs for the computed number of frames
        for r in range(nbFrames):
            output_file.write(held_keyboard_state)

        # Build new input string
        pressed_keyboard_state = ''
        held_keyboard_state = ''

        input_nums = ['9', '10', '11', '12', '14', '15', '16', '17', '19', '20']
        input_strs = ['up', 'down', 'left', 'right', 'jump', 'dash', 'henry', 'hat', 'pause', 'restart']
        # Some inputs must only be pressed once, and some must be held until next command
        held_inputs = [True, True, True, True, True, False, False, False, False, False]

        mapped_keys = ['ff52', 'ff54', 'ff51', 'ff53', '7a', '78', '63', '76', 'ff1b', '72']

        for single_input in match.group(2).split(',')[1:]:
            for i in range(len(input_nums)):
                if single_input == input_nums[i] or single_input.lower() == input_strs[i].lower():
                    pressed_keyboard_state += mapped_keys[i] + ':'
                    if held_inputs[i]:
                        held_keyboard_state += mapped_keys[i] + ':'

        # Remove trailing ':'
        if pressed_keyboard_state:
            pressed_keyboard_state = pressed_keyboard_state[:-1]
        if held_keyboard_state:
            held_keyboard_state = held_keyboard_state[:-1]

        # Write the constructed input line
        pressed_keyboard_state = '|' + pressed_keyboard_state + '|\n'
        held_keyboard_state = '|' + held_keyboard_state + '|\n'

        # Write one input line because we expect the file to not write unused inputs
        output_file.write(pressed_keyboard_state)
        frame += 1

# The frame count of the last inputs is not specified. Write it 30 times
for r in range(30):
    output_file.write(held_keyboard_state)

input_file.close()
output_file.close()
