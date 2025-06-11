#!/usr/bin/env python3

import math
import os
import io
from PIL import Image
import argparse
from pathlib import Path


parser = argparse.ArgumentParser("zeal2gif")
parser.add_argument("-t","--tileset", help="Zeal Tileset (ZTS)", required=True)
parser.add_argument("-p", "--palette", help="Zeal Palette (ZTP)", required=True)
parser.add_argument("-o", "--output", help="Output GIF Filename")
parser.add_argument("-b", "--bpp", help="Bits Per Pixel", type=int, default=8, choices=[1,2,4,8])
parser.add_argument("-z", "--compressed", help="Decompress RLE", action="store_true")
parser.add_argument("-s", "--show", help="Open in Viewer", action="store_true")
parser.add_argument("-v", "--verbose", help="Verbose output", action='store_true')
parser.add_argument("-d", "--debug", help="Debug output", action='store_true')

tile_width = 16
tile_height = 16
tiles_per_row = 16

def create_dir(file_path):
  if "." in os.path.basename(file_path):
      directory = os.path.dirname(file_path)
  else:
      directory = file_path
  if directory and not os.path.exists(directory):
      os.makedirs(directory, exist_ok=True)

def get_tilesheet_size(tiles):
  width = 1
  height = 1
  count = len(tiles)
  # max 16 tiles across
  if count > tiles_per_row:
    width = tiles_per_row
    height = math.ceil(len(tiles) / tiles_per_row)
  else:
    width = count
    height = 1

  return (width * tile_width,height*tile_height)

def RGB565toRGB(value):
  r = int((((value >> 11) & 0x1F) * 255 + 15) / 31)
  g = int((((value >> 5) &0x3F) * 255 + 31) / 63)
  b = int((((value & 0x1F)) * 255 + 15) / 31)
  return [r,g,b]

def getPalette(paletteFile):
  pf = open(paletteFile, "rb")
  pb = pf.read()
  palette = []
  for i in range(0, len(pb), 2):
    lo = pb[i]
    hi = pb[i + 1]
    value = (hi << 8) + lo
    palette += RGB565toRGB(value)

  # print("PALETTE: [{}]".format(", ".join(hex(x) for x in palette)))
  return palette

def makeSprite(pixels, palette: list):
  image = Image.new(
    mode="P", # mode
    size=(tile_width, tile_height), # size
  )
  image.putpalette(palette, rawmode="RGB")
  for y in range(0, tile_height):
    for x in range(0, tile_width):
      index = (y * tile_height) + x
      # print("tile height", tile_height, "y", y, "x", x, "index", index, "len", len(pixels))
      if(index >= len(pixels)):
        break
      image.putpixel(
        (x,y), # xy
        pixels[index] # value
      )
  return image

def getSpritePixels(bpp, tile):
  sprites = []

  if bpp == 1:
    # one bit per pixel
    pixels = bytearray()
    for byte in tile:
      for bit in reversed(range(8)):
        p = ((byte >> bit) & 1)
        pixels.append(p)
    sprites.append(pixels)

  elif bpp == 2:
    # two-bits per pixel
    pixels = bytearray()
    for byte in tile:
      pixels.append((byte >> 6) & 3)
      pixels.append((byte >> 4) & 3)
      pixels.append((byte >> 2) & 3)
      pixels.append((byte >> 0) & 3)
    sprites.append(pixels)

  elif bpp <= 4:
    # one nibble per pixel
    pixels = bytearray()
    for byte in tile:
      p1,p2 = (byte >> 4) & 0x0F, byte & 0x0F
      pixels.append(p1)
      pixels.append(p2)
    sprites.append(pixels)

  else:
    pixels = bytearray(tile)
    sprites.append(pixels)

  return sprites

def decompress(data):
  ret = []
  length = 0
  value = None
  i = 0
  len_of_data = len(data) - 1
  while i < len_of_data:
    length = data[i]
    i += 1 # every other byte
    if length >= 0x80:
      length = (length - 0x80) + 1
      value = data[i].to_bytes(1,'big')
      ret += value * length
      i += 1
    else:
      length = length + 1
      ret += data[i:i+length]
      i += length

  return bytes(ret)

def convert(args):
  palette = getPalette(args.palette)

  data = None
  with open(args.tileset, mode="rb") as f:
    data = f.read()
    if(args.compressed):
      data = decompress(data)
    data = io.BytesIO(data)

  tiles = []
  image_count = 0

  tilesize_bpp = 0
  if args.bpp == 1:
    tilesize_bpp = (tile_width * tile_height) // 8
  elif args.bpp == 2:
    tilesize_bpp = (tile_width * tile_height) // 4
  elif args.bpp <= 4:
    tilesize_bpp = (tile_width * tile_height) // 2
  else:
    tilesize_bpp = tile_width * tile_height

  tile = data.read(tilesize_bpp)

  while tile:
    if len(tile) < tilesize_bpp:
      break

    images = []
    for pixels in getSpritePixels(args.bpp, tile):
      images.append(makeSprite(pixels, palette))

    for image in images:
      tiles.append(image)
      image_count += 1

    tile = data.read(tilesize_bpp)

  tilesheet_size = get_tilesheet_size(tiles)
  spritesheet = Image.new(mode="P", size=tilesheet_size)
  spritesheet.putpalette(palette, rawmode="RGB")

  sprites_x = int(tilesheet_size[0]/tile_width)
  sprites_y = int(tilesheet_size[1]/tile_height)
  for y in range(0, sprites_y):
    for x in range(0,sprites_x):
      index = (y*sprites_x) + x
      if 0 <= index < len(tiles):
        # print("sprite index", index)
        tile = tiles[index]
        spritesheet.paste(tile, (x * tile_width,y*tile_height))

  if args.debug:
    print("columns", sprites_x, "rows", sprites_y, "tiles", image_count)
  return spritesheet

def main():
  args = parser.parse_args()
  if args.verbose:
    print("args", args)
  output = convert(args)

  outputFile = args.output
  if outputFile == None:
    outputFile = Path(args.tileset).with_suffix(".gif")

  create_dir(outputFile)

  if args.verbose:
    print("output", outputFile)

  output.save(
    outputFile,
    format="GIF",
    optimize=False
  )
  if(args.show):
    output.show()

if __name__ == "__main__":
  main()
