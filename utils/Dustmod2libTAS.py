#!/usr/bin/env python
# This script converts Dustmod files
# (https://dustkidblog.wordpress.com/2016/08/07/nexus-tasing-with-dustmod/) to libTAS input file.
# Just run ./Dustmod2libTAS path/to/tasfile

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

regex_input = re.compile(r'([01]+)')
regex_command = re.compile(r'(LOAD|SYNCLOAD|STATS|INCLUDE)')
regex_mouse = re.compile(r'MOUSE:([\d]*):([\d]*):([01]*)')

mouse_state = ''
prev_input_string = '000000000'
prev_input = -1
prev_input_frame = -1
frame = 0

for line in input_file:
    if regex_command.match(line):
        continue
    if regex_mouse.match(line):
        mouse_match = regex_mouse.match(line)
        mouse_state = mouse_match.group(1) + ':' + mouse_match.group(2) + ':'
        mouse_button_input = mouse_match.group(3)
        for b in range(5):
            if mouse_button_input[b] == '1':
                mouse_state += str(b)
            else:
                mouse_state += '.'
        continue

    match = regex_input.match(line)
    if match:

        input_string = match.group(1)
#        mapped_keys = ['ff51', 'ff53', 'ff52', 'ff54', '7a', '62', '78', '63', 'ff1b']
        mapped_keys = ['61', '63', '62', '64', '68', '66', '67', '65', 'ff1b']
        keyboard_state = ''

        remove_dtd = False
        for i in [K_ESC, K_LEFT, K_UP, K_RIGHT, K_DOWN, K_HEAVY, K_DASH, K_LIGHT, K_JUMP]:
            # Check if input i is pressed
            if input_string[i] == '1' and prev_input_string[i] == '0':

                # Check if the same input was already pressed
                if prev_input == i:

                    # Check if double tap input
                    if i == K_LEFT or i == K_RIGHT or i == K_DOWN:
                        if (frame - prev_input_frame) < 15:
                            if input_string[K_DASH] == '1' and prev_input_string[K_DASH] == '0':
                                # Remove double tap dash input
                                print "Remove DTD"
                                remove_dtd = True
                prev_input = i
                prev_input_frame = frame

        prev_input_string = input_string

        for i in range(9):
            if input_string[i] == '1':
                if i != K_DASH or not remove_dtd:
                    keyboard_state += mapped_keys[i] + ':'

        # Remove trailing ':'
        if keyboard_state:
            keyboard_state = keyboard_state[:-1]

        # Write the constructed input line
        output_line = '|' + keyboard_state + '|'
        if mouse_state:
            output_line += mouse_state
            mouse_state = ''
        else:
            output_line += '0:0:.....'

        output_line += '|\n'
        output_file.write(output_line)

        frame += 1

input_file.close()
output_file.close()
