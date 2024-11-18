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
parser.add_argument("-c", "--compress", help="Compress with RLE", action="store_true")
parser.add_argument("-s", "--strip", help="Strip N tiles off the end", type=int, default=0)

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

def _compress_same_seq(data):
  count = 0
  i = 0
  while (i < len(data)) and (data[0] == data[i] and count < 128):
    count += 1
    i += 1

  return count

def _compress_diff_seq(data):
  count = 0
  i = 1
  while (i < len(data)) and (data[i-1] != data[i] and count < 128):
    count += 1
    i += 1

  return count

def compress(tile: list):
  ret = []
  i = 0
  while i < len(tile): # TILE_SIZE # 16 * 16
    same_count = _compress_same_seq(tile[i:])
    diff_count = _compress_diff_seq(tile[i:])

    if diff_count > 0:
      ret.append(diff_count - 1)
      for j in range(i, i+diff_count):
        ret.append(tile[j])
      i += diff_count
    else:
      ret.append((same_count - 1) + 0x80)
      ret.append(tile[i])
      i += same_count

  return ret

def convert(args):
  gif = Image.open(args.input)
  palette = getPalette(gif)
  # print("palette", palette)

  tiles = []
  tiles_per_row = int(gif.width / tile_width)
  rows = int(gif.height / tile_height)
  total_tiles = tiles_per_row * rows
  if args.strip > 0:
    total_tiles -= args.strip


  for y in range(0, rows):
    for x in range(0, tiles_per_row):
      index = (y * tiles_per_row) + x
      if index >= total_tiles:
        continue # reached the end
      # print("tile", x, y)
      ox = (x * tile_width)
      oy = (y * tile_height)
      tile = gif.crop((ox, oy, ox + tile_width, oy + tile_height))
      pixels = list(tile.getdata())

      if(args.bpp == 4):
        op = pixels.copy()
        pixels = []
        for idx in range(0, len(op), 2):
          p1 = op[idx]
          p2 = op[idx+1]
          p1 <<= 4
          pixels.append(p1 + p2)


      if(args.compress):
        pixels = compress(pixels)
      tiles += pixels

  print("columns", tiles_per_row, "rows", rows, "tiles", (tiles_per_row * rows) - args.strip)
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