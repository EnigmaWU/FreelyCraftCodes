# Open data file and convert to FlameGraph format, and default output to stdout
# Line-Format in data file:
#   <call stack 0>:...:<call stack n>: <count>
# Output format is compatible with FlameGraph 

import sys
import os
import re
import argparse

parser = argparse.ArgumentParser(description='Convert call stack data to FlameGraph format')
parser.add_argument('-i', '--input', dest='input', default='-', help='input file (default: stdin)')
parser.add_argument('-o', '--output', dest='output', default='-', help='output file (default: stdout)')

args = parser.parse_args()
if args.input == '-':
    input = sys.stdin
else:   
    input = open(args.input, 'r')

if args.output == '-':
    output = sys.stdout
else:
    output = open(args.output, 'w')

# Define a function to append call stack in line format 'f0 f1 ... fn weightValue',
#  and convert to FlameGraph format
def appendCallStack(CallStacks, WeightValue):
    # Split call stack into individual frames
    frames = CallStacks.split(':')
    # Output call stack in FlameGraph format
    output.write(';'.join(frames) + ' ' + WeightValue + '\n')

# Read input file line-by-line, and convert to FlameGraph format
def readFileLineByLine(input):
    for line in input:
        line = line.strip()
        if line == '':
            continue
        # Split line of format 'f0 f1 ... fn weightValue'
        (CallStacks,WeightValue) = line.rsplit(' ', 1)
        appendCallStack(CallStacks, WeightValue)

# main

