#!/usr/bin/env python3

import math
from functools import reduce
from PIL import Image
import argparse
from pathlib import Path


parser = argparse.ArgumentParser("gif2zeal")
parser.add_argument("-i", "--input", help="Input GIF Filename", required=True)
parser.add_argument("-t","--tileset", help="Zeal Tileset (ZTS)")
parser.add_argument("-p", "--palette", help="Zeal Palette (ZTP)")
parser.add_argument("-b", "--bpp", help="Bits Per Pixel", type=int, default=8, choices=[1,4,8])
parser.add_argument("-c", "--compressed", help="Compress with RLE", action="store_true")

tile_width = 16
tile_height = 16

def RGBtoRGB565(r,g,b):
  red = (r >> 3) & 0x1F
  green = (g >> 2) & 0x3F
  blue = (b >> 3) & 0x1F
  return (red << 11) | (green << 5) | blue

def getPalette(gif):
  if not gif.mode == "P":
    print("Invalid Mode, expected P and got ", gif.mode)
    exit(2)
  palette = gif.getpalette() # rawmode="BGR;16")
  if palette == None:
    print("invalid GIF palette")
    exit(2)

  result = []

  for x in range(0, len(palette), 3):
    # print("x", x)
    # print("x", x, end="   ")
    r = palette[x]
    g = palette[x+1]
    b = palette[x+2]
    color = reduce(lambda acc, x: (acc << 8) + x, [r,g,b])
    # print("rgb   ", str(r).rjust(3), str(g).rjust(3), str(b).rjust(3), "#{:06x}".format(color), end=" | ")
    color = RGBtoRGB565(r,g,b)
    # print("rgb565", "#{:06x}".format(color), str(color).rjust(5))

    lo = color & 0xFF
    hi = (color >> 8) & 0xFF

    result.append(lo)
    result.append(hi)
    # print("[{}]".format(", ".join(hex(x) for x in palette)))
  return result


def convert(args):
  gif = Image.open(args.input)
  palette = getPalette(gif)
  # print("palette", palette)

  tiles = []
  tiles_per_row = int(gif.width / tile_width)
  rows = int(gif.height / tile_height)

  print("columns", tiles_per_row, "rows", rows, "tiles", tiles_per_row * rows)

  for y in range(0, rows):
    for x in range(0, tiles_per_row):
      # print("tile", x, y)
      ox = (x * tile_width)
      oy = (y * tile_height)
      tile = gif.crop((ox, oy, ox + tile_width, oy + tile_height))
      # print("crop", ox, oy, ox + tile_width, oy + tile_height)
      pixels = list(tile.getdata())
      tiles = tiles + pixels
      # print("pixels", pixels)

  return (tiles, palette)

def main():
  args = parser.parse_args()
  print("args", args)
  tileset, palette = convert(args)

  tilesetFileName = args.tileset
  if tilesetFileName == None:
    tilesetFileName = Path(args.input).with_suffix(".zts")
  paletteFileName = args.palette
  if paletteFileName == None:
    paletteFileName = Path(args.input).with_suffix(".ztp")

  print("tileset", tilesetFileName) #, tileset)
  print("palette", paletteFileName) #, palette)

  # tilesetFile = bytearray(tileset)
  # paletteFile = bytearray(palette)

  with open(tilesetFileName, "wb") as file:
    file.write(bytearray(tileset))

  with open(paletteFileName, "wb") as file:
    file.write(bytearray(palette))

if __name__ == "__main__":
  main()