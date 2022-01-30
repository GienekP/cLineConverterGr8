#!/bin/sh

convert -depth 8 picture.png rgb:picture.rgb
rgb2atari picture.rgb ataripal.raw 40
rm picture.rgb
convert -colors 8 gray.png gray:mono.raw
lineconvg8 ataripal.raw mono.raw picture.asm 2
rm ataripal.raw
rm mono.raw
mads picture.asm -o:picture.xex
