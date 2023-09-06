#!/usr/bin/env python
# This script converts tas files from Hourglass format to libTAS input file.
# Just run ./hourglass2libTAS path/to/tasfile.tas

import re
import sys
import os
import math

input_file = open(sys.argv[1], 'rb')
output_file = open(os.path.dirname(sys.argv[1])+'/inputs' , 'w')

input_keys = [0x0D, 0xA0, 0xA2, 0x1B, 0x20, 0x25, 0x26, 0x27, 0x28] + list(range(0x41, 0x5B))
mapped_keys = ["ff0d", "ffe1", "ffe3", "ff1b", "20", "ff51", "ff52", "ff53", "ff54"] + \
    ["61", "62", "63", "64", "65", "66", "67", "68", "69", "6a", "6b", "6c", "6d", "6e", "6f"] + \
    ["70", "71", "72", "73", "74", "75", "76", "77", "78", "79", "7a", "7b"]

input_file.seek(1024, 0)

byte = input_file.read(1)
while byte != b"":
    keyboard_state = ''
    for k in range(8):
        i = 0
        if (byte[0] != 0):
            try:
                i = input_keys.index(byte[0])
            except ValueError:
                print("Unknown VK key " + str(byte[0]))
            else:
                keyboard_state += mapped_keys[i] + ':'
        byte = input_file.read(1)

    # Remove trailing ':'
    if keyboard_state:
        keyboard_state = keyboard_state[:-1]

    # Write the constructed input line
    keyboard_state = '|K' + keyboard_state + '|\n'

    output_file.write(keyboard_state)

input_file.close()
output_file.close()
