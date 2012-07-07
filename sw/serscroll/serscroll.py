#!/usr/bin/python2

import pymouse
import sys, serial

# initialize mouse
mouse = pymouse.PyMouse();

cxn = serial.Serial('/dev/ttyUSB0', 230400);
while True:
    c = cxn.read(1);
    if c == '>':
        offs = 3;
    elif c == '<':
        offs = 2;
    else:
        continue;

    x,y = mouse.position();
    mouse.press(x,y,4+offs);
    mouse.release(x,y,4+offs);
