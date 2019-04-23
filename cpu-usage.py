#!/usr/bin/env python3
import sys

with open(sys.argv[1]) as infile:
    lines = infile.readlines()
    i = 1
    while i < len(lines):
        x = float(lines[i].lstrip().split(' ')[0])
        if x:
            print(x)
            i += 4
        else:
            print(x)
            i += 3
