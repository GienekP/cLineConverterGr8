#!/bin/sh

convert -depth 8 picture.png rgb:picture.rgb
rgb2atari picture.rgb ataripal.raw 20
rm picture.rgb
convert -colors 2 -monochrome -type bilevel mono.png gray:mono.raw
lineconvg8 ataripal.raw mono.raw picture.asm 2C
rm ataripal.raw
rm mono.raw
mads picture.asm -o:picture.xex
