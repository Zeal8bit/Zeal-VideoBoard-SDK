#!/usr/bin/env python3

import os
from functools import reduce
from PIL import Image
import argparse
from pathlib import Path


parser = argparse.ArgumentParser("gif2zeal")
parser.add_argument("-i", "--input", help="Input GIF Filename", required=True)
parser.add_argument("-t", "--tileset", help="Zeal Tileset (ZTS)")
parser.add_argument("-p", "--palette", help="Zeal Palette (ZTP)")
parser.add_argument("-o", "--output", help="Output path, can be just a path")
parser.add_argument("-b", "--bpp", help="Bits Per Pixel", type=int, default=8, choices=[1,4,8])
parser.add_argument("-z", "--compress", help="Compress with RLE", action="store_true")
parser.add_argument("-s", "--strip", help="Strip N tiles off the end", type=int, default=0)
parser.add_argument("-c", "--colors", help="Max Colors in Palette", type=int, default=256)
parser.add_argument("-v", "--verbose", help="Verbose output", action='store_true')
parser.add_argument("-d", "--debug", help="Debug output", action='store_true')

tile_width = 16
tile_height = 16

def create_dir(file_path):
  if "." in os.path.basename(file_path):
      directory = os.path.dirname(file_path)
  else:
      directory = file_path
  if directory and not os.path.exists(directory):
      os.makedirs(directory, exist_ok=True)

def process_paths(input_path, output_path):
  # Extract input directory and filename
  input_dir = os.path.dirname(input_path)
  input_filename = os.path.basename(input_path)

  # Check if output path has an extension (meaning it's a file) or is a directory
  output_has_extension = "." in os.path.basename(output_path)

  if output_has_extension:
      output_dir = os.path.dirname(output_path)
      output_filename = os.path.basename(output_path)
  else:
      output_dir = output_path  # Assume it's a directory
      output_filename = os.path.splitext(input_filename)[0] + ".zts"

  # Construct full output path
  full_output_path = os.path.join(output_dir, output_filename)

  return output_dir, output_filename, full_output_path

def RGBtoRGB565(r,g,b):
  red = (r >> 3) & 0x1F
  green = (g >> 2) & 0x3F
  blue = (b >> 3) & 0x1F
  return (red << 11) | (green << 5) | blue

def getPalette(args, gif):
  if not gif.mode == "P":
    print("Invalid Mode, expected P and got ", gif.mode)
    exit(2)
  palette = gif.getpalette() # rawmode="BGR;16")
  if palette == None:
    print("invalid GIF palette")
    exit(2)

  result = []

  max_colors = args.colors
  if(args.bpp == 1):
    max_colors = 2
  if(args.bpp == 4):
    max_colors = 16

  for x in range(0, min(max_colors * 3, len(palette)), 3):
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
  palette = getPalette(args, gif)
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

      if(args.bpp == 1):
        op = pixels.copy()
        pixels = []
        for idx in range(0, len(op), 8):
          b =  (op[idx+0] & 1) << 7
          b |= (op[idx+1] & 1) << 6
          b |= (op[idx+2] & 1) << 5
          b |= (op[idx+3] & 1) << 4
          b |= (op[idx+4] & 1) << 3
          b |= (op[idx+5] & 1) << 2
          b |= (op[idx+6] & 1) << 1
          b |= (op[idx+7] & 1) << 0
          pixels.append(b)


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

  if args.debug:
    print("columns", tiles_per_row, "rows", rows, "tiles", (tiles_per_row * rows) - args.strip)
  return (tiles, palette)

def parse_filename_flags(args):
  input = args.input
  split = input.rsplit("__", 1)
  if len(split) < 2:
    return args

  filename = split[0]
  (flags, extension) = split[1].rsplit(".", 1)

  tileset = args.tileset
  palette = args.palette
  bpp = args.bpp
  compress = args.compress
  colors = args.colors
  strip = args.strip

  if tileset == None:
    tileset = Path(filename).with_suffix(".zts")
  if palette == None:
    palette = Path(filename).with_suffix(".ztp")

  i = 0
  while i < len(flags):
    flag = flags[i]
    match flag:
      case 'B': # BPP
        bpp = int(flags[i+1], 0)
        i += 1
      case 'C' | 'P': # palette
        h1 = flags[i+1]
        h2 = flags[i+2]
        colors = int(h1 + h2, 16)
        i += 2
      case 'Z': # compress
        compress = True
      case 'S': # strip
        h1 = flags[i+1]
        h2 = flags[i+2]
        strip = int(h1 + h2, 16)
        i += 2
    i += 1


  if args.debug:
    print("parser", input, filename, flags, extension)
  return argparse.Namespace(
    input=input,
    tileset=tileset,
    palette=palette,
    bpp=bpp,
    compress=compress,
    colors=colors,
    strip=strip
  )

def main():
  args = parser.parse_args()
  args = parse_filename_flags(args)
  if args.verbose:
    print("args", args)


  tileset, palette = convert(args)

  outputDir, outputFilename, outputPath = process_paths(args.input, args.output)
  if args.debug:
    print("outputDir", outputDir)
    print("outputFilename", outputFilename)
    print("outputPath", outputPath)

  create_dir(outputPath)

  tilesetFileName = args.tileset
  if tilesetFileName == None:
    tilesetFileName = Path(outputPath).with_suffix(".zts")
  paletteFileName = args.palette
  if paletteFileName == None:
    paletteFileName = Path(outputPath).with_suffix(".ztp")

  if args.verbose:
    print("tileset", tilesetFileName) #, tileset)
    print("palette", paletteFileName) #, palette)

  with open(tilesetFileName, "wb") as file:
    file.write(bytearray(tileset))

  create_dir(paletteFileName)
  with open(paletteFileName, "wb") as file:
    file.write(bytearray(palette))

if __name__ == "__main__":
  main()
