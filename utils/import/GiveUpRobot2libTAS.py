#!/usr/bin/env python
# This script converts tas files from Give Up Robot format to libTAS input file.
# Just run ./GiveUpRobot2libTAS path/to/tasfile.tas

import re
import sys
import os
import math

input_file = open(sys.argv[1], 'r')
output_file = open(os.path.splitext(sys.argv[1])[0]+'.ltm' , 'w')

regex_input = re.compile(r'[\s]*([\d]*)((?:,(?:[RLUDJKXCGSQNF]|[\d]*))*)')
regex_comment = re.compile(r'[\s]*(#|[\s]*$)')

intput_strs = "^<>+Z~"
mapped_keys = ['ff52', 'ff51', 'ff53', 'ff54', '77', 'ff0d']

for line in input_file:
    keyboard_state = ''

    for i in range(len(intput_strs)):
        if line.find(intput_strs[i]) != -1:
            keyboard_state += mapped_keys[i] + ':'

    # Remove trailing ':'
    if keyboard_state:
        keyboard_state = keyboard_state[:-1]

    # Write the constructed input line
    keyboard_state = '|K' + keyboard_state + '|\n'

    output_file.write(keyboard_state)

input_file.close()
output_file.close()
