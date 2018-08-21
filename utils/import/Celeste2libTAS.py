#!/usr/bin/env python
# This script converts tas files from Celeste TAS tool
# (https://github.com/ShootMe/CelesteTAS/) to libTAS input file.
# Just run ./Celeste2libTAS path/to/tasfile.tas

import re
import sys
import os
import math

input_file = open(sys.argv[1], 'r')
output_file = open(os.path.splitext(sys.argv[1])[0]+'.ltm' , 'w')

regex_input = re.compile(r'[\s]*([\d]*)((?:,(?:[RLUDJKXCGSQNF]|[\d]*))*)')
regex_comment = re.compile(r'[\s]*(#|[\s]*$)')

for line in input_file:
    if regex_comment.match(line):
        continue
    match = regex_input.match(line)
    if match:

        button_order   = "ABXYbgs()[]udlr"
        output_buttons = list("...............")

        button_mapping = "JCXK..S...GUDLR"
        output_axes = "0:0"

        is_axis = False
        for single_input in match.group(2).split(',')[1:]:

            if is_axis:
                if single_input == '':
                    angle = 0
                else:
                    angle = int(single_input)

                # Compute coordinates of the left analog stick to match the
                # requested angle. Use the max amplitude to get precise values
                rad_angle = math.radians(angle)
                x = 32767 * math.sin(rad_angle)
                y = 32767 * math.cos(rad_angle)
                output_axes = str(int(x)) + ':' + str(int(y))

                is_axis = False
                continue

            if single_input == 'F':
                is_axis = True
                continue

            # Look at the mapping of the action
            mapped_index = button_mapping.find(single_input)
            output_buttons[mapped_index] = button_order[mapped_index]

        # Write the constructed input line
        output_line = '||' + output_axes + ':0:0:0:0:' + ''.join(output_buttons) + '|\n'

        for n in range(int(match.group(1))):
            output_file.write(output_line)

input_file.close()
output_file.close()
