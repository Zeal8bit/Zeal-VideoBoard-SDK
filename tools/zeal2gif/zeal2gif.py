#!/usr/bin/env python3

import math
from PIL import Image
import argparse
from pathlib import Path


parser = argparse.ArgumentParser("zeal2gif")
parser.add_argument("-t","--tileset", help="Zeal Tileset (ZTS)", required=True)
parser.add_argument("-p", "--palette", help="Zeal Palette (ZTP)", required=True)
parser.add_argument("-o", "--output", help="Output GIF Filename")
parser.add_argument("-b", "--bpp", help="Bits Per Pixel", type=int, default=8, choices=[1,4,8])
parser.add_argument("-c", "--compressed", help="Decompress RLE", action="store_true")
parser.add_argument("-s", "--show", help="Open in Viewer", action="store_true")

tile_width = 16
tile_height = 16
tiles_per_row = 16

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

  print("width", width, "height", height)
  return (width * tile_width,height*tile_height)

def RGB565toRGB(rgb565, palette):
  # print("palette", rgb565)
  lo = palette[rgb565 * 2]
  hi = palette[rgb565 * 2 + 1]
  # print("  color", hi, lo)
  return (hi,lo)

def getPalette(paletteFile):
  pf = open(paletteFile, "rb")
  pb = pf.read()
  palette = []
  for b in pb:
    palette.append(b)
  return palette

def makeSprite(pixels):
  # print("pixels", pixels)
  image = Image.frombytes(
    "RGB", # mode
    (tile_width,tile_height), # size
    pixels, # data
    "raw", # decoder name
    "BGR;16",
    0, 1
    )
  return image

def getSpritePixels(bpp, tile, palette):
  sprites = []

  if bpp == 1:
    # one bit per pixel
    for pixel in tile:
      pixels = bytearray()
      for b in range(8):
        p = (b >> 1) & 1
        (hi,lo) = RGB565toRGB(p, palette)
        pixels.append(lo)
        pixels.append(hi)
      sprites.append(pixels)

  elif bpp <= 4:
    # one nibble per pixel
    pixels = bytearray()
    for pixel in tile:
      p1,p2 = pixel >> 4, pixel & 0x0F

      (hi,lo) = RGB565toRGB(p1, palette)
      pixels.append(lo)
      pixels.append(hi)

      (hi,lo) = RGB565toRGB(p2, palette)
      pixels.append(lo)
      pixels.append(hi)
    sprites.append(pixels)

  else:
    # one byte per pixel
    pixels = bytearray()
    for pixel in tile:
      (hi,lo) = RGB565toRGB(pixel, palette)
      pixels.append(lo)
      pixels.append(hi)
    sprites.append(pixels)

  return sprites


def convert(args):
  palette = getPalette(args.palette)

  f = open(args.tileset, mode="rb")
  tiles = []
  image_count = 0

  tilesize_bpp = 0
  if args.bpp == 1:
    tilesize_bpp = 32
  elif args.bpp <= 4:
    tilesize_bpp = 128
  else:
    tilesize_bpp = tile_width * tile_height

  tile = f.read(tilesize_bpp)

  while tile:
    print("tile len", len(tile), tilesize_bpp)
    if len(tile) < tilesize_bpp:
      break

    images = []
    for pixels in getSpritePixels(args.bpp, tile, palette):
      images.append(makeSprite(pixels))

    print("images", len(images))
    for image in images:
      tiles.append(image)
      image_count += 1

    tile = f.read(tilesize_bpp)

  f.close()

  tilesheet_size = get_tilesheet_size(tiles)
  spritesheet = Image.new(mode="RGB", size=tilesheet_size)

  sprites_x = int(tilesheet_size[0]/tile_width)
  sprites_y = int(tilesheet_size[1]/tile_height)
  print("sprites_x", sprites_x, "sprites_y", sprites_y, "tiles", len(tiles))
  for y in range(0, sprites_y):
    for x in range(0,sprites_x):
      index = (y*sprites_x) + x
      if 0 <= index < len(tiles):
        print("sprite index", index)
        tile = tiles[index]
        spritesheet.paste(tile, (x * tile_width,y*tile_height))

  print("sprite count", image_count)
  return spritesheet

def main():
  args = parser.parse_args()
  print("args", args)
  output = convert(args)

  outputFile = args.output
  if outputFile == None:
    outputFile = Path(args.tileset).with_suffix(".gif")
  print("output", outputFile)
  output.save(outputFile, "GIF")
  if(args.show):
    output.show()

if __name__ == "__main__":
  main()