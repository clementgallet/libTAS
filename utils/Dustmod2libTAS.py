#!/usr/bin/env python
# This script converts Dustmod files
# (https://dustkidblog.wordpress.com/2016/08/07/nexus-tasing-with-dustmod/) to libTAS input file.
# Just run ./Dustmod2libTAS path/to/tasfile

import re
import sys
import os
import math

input_file = open(sys.argv[1], 'r')
output_file = open(os.path.splitext(sys.argv[1])[0]+'.ltm' , 'w')

regex_input = re.compile(r'([01]+)')
regex_command = re.compile(r'(LOAD|SYNCLOAD|STATS|INCLUDE)')
regex_mouse = re.compile(r'MOUSE:([\d]*):([\d]*):([01]*)')

mouse_state = ''

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
        mapped_keys = ['ff51', 'ff53', 'ff52', 'ff54', '7a', '62', '78', '63', 'ff1b']
#        mapped_keys = ['61', '63', '62', '64', '68', '66', '67', '65', 'ff1b']
        keyboard_state = ''

        for i in range(9):
            if input_string[i] == '1':
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

input_file.close()
output_file.close()
