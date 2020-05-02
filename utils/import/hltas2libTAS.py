#!/usr/bin/env python
# This script converts hltas files (https://github.com/hltas) to libTAS input file.
# Just run ./hltas2libTAS path/to/tasfile

import re
import sys
import os
import math

input_file = open(sys.argv[1], 'r')
output_file = open(os.path.splitext(sys.argv[1])[0]+'.ltm' , 'w')

regex_input = re.compile(r'----------\|([a-z\-]+)\|([a-z\-]+)\|([\d\.]+)\|([\d\.]+)\|([\d\.]+)\|')

previous_yaw = 361
previous_pitch = 361

def angle_step(angle):
    return int(angle * 65536 / 360) & 65535

def angle_mod(angle):
    return (360.0 / 65536) * (int(angle * 65536 / 360) & 65535)

for line in input_file:
    match = regex_input.match(line)
    if match:

        # Direction inputs
        dir_string = match.group(1)
        dir_keys = ['77', '61', '64', '73', 'ff55', 'ff56']
        keyboard_state = ''

        for i in range(6):
            if dir_string[i] != '-':
                keyboard_state += dir_keys[i] + ':'

        # Action inputs
        action_string = match.group(2)
        action_keys = ['20', 'ffe3', '65', '1', '3', '72']

        for i in [0, 1, 2, 5]:
            if action_string[i] != '-':
                keyboard_state += action_keys[i] + ':'

        # Remove trailing ':'
        if keyboard_state:
            keyboard_state = keyboard_state[:-1]

        mouse_state = ''

        # Yaw
        yaw = float(match.group(4))
        if previous_yaw == 361:
            previous_yaw = yaw

        # Compute a rough value of x
        delta_yaw = yaw - previous_yaw

        if delta_yaw < -180:
            delta_yaw += 360
        if delta_yaw > 180:
            delta_yaw -= 360

        delta_x = int(delta_yaw / (0.2 * 0.022))

        # Tweak the value
        yaw_step = angle_step(yaw)

        predicted_yaw = (delta_x * 0.2 * 0.022) + angle_mod(previous_yaw)
        predicted_step = angle_step(predicted_yaw)

        while predicted_step != yaw_step:
            # Check wrapping
            if (predicted_step - yaw_step) > 32768:
                predicted_step -= 65536
            if (yaw_step - predicted_step) > 32768:
                predicted_step += 65536

            if predicted_step > yaw_step:
                delta_x -= 1
            if predicted_step < yaw_step:
                delta_x += 1
            predicted_yaw = (delta_x * 0.2 * 0.022) + angle_mod(previous_yaw)
            predicted_step = angle_step(predicted_yaw)

        previous_yaw = yaw

        mouse_state += str(-delta_x) + ':'

        # Pitch
        pitch = float(match.group(5))
        if previous_pitch == 361:
            previous_pitch = pitch

        delta_pitch = pitch - previous_pitch
        delta_y = int(delta_pitch / (0.2 * 0.022))
        previous_pitch = pitch

        mouse_state += str(delta_y) + ':R:'

        # Mouse actions
        if action_string[3] != '-':
            mouse_state += '1.'
        else:
            mouse_state += '..'

        if action_string[4] != '-':
            mouse_state += '3..'
        else:
            mouse_state += '...'

        # Write the constructed input line
        output_line = '|' + keyboard_state + '|' + mouse_state + '|'

        output_line += '|\n'
        output_file.write(output_line)

input_file.close()
output_file.close()
