# PSXFunkin Formats

## TIM files

In [iso/](/iso/), you can find .png files with .txt files alongside them. This txt file describes 5 things.

`TextureX TextureY PaletteX PaletteY BPP`

The texture and palette positions are in VRAM, the texture positions are expected to be TPage aligned, where TPages are 64x256, and there are 16 TPages horizontally, and 2 TPages vertically, meaning there's a 1024x512 VRAM.

The top left 320x480 area of VRAM is dedicated to the framebuffers. The bottom left 256x32 area of VRAM is dedicated to palettes.

BPP can be 4 or 8, where 4bpp can have 16 colours (including transparency if any), and 8bpp can have 256 colours (including transparency if any).

Textures should only be up to 256x256, which for 4bpp is 1x1 TPages, and for 8bpp is 2x1 TPages.

You should keep TPage and VRAM space in mind when positioning them. Look at the default included txt files for reference.

TIMs should be packed into .arc files, and you can control the dependencies and rules of .tim conversion and packing in [Makefile.tim](/Makefile.tim).

## XA files

In [iso/music/](/iso/music/), you can find .ogg files with .txt files for various groups of .xa files. The txt files are pretty obvious, so I won't go into much more detail here.

You can control the dependencies and rules of .xa conversion and interleaving in [Makefile.xa](/Makefile.xa).

You must change the lengths in [/src/audio_def.h](audio_def.h) if you modify the oggs.

## CHT files

In [iso/chart/](/iso/chart/), you can find .json files. These .json files will be converted to .cht files that are significantly smaller and can be played by the game.

## What files go into the final binary

You can control which files go into the final binary in [funkin.xml](/funkin.xml). The format is pretty obvious, so I won't go into much more detail here.
