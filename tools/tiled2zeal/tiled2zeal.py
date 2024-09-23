#!/usr/bin/env python3

import argparse
from pathlib import Path
import xml.etree.ElementTree as ET

parser = argparse.ArgumentParser("gif2zeal")
parser.add_argument("-i", "--input", help="Input GIF Filename", required=True)
parser.add_argument("-m", "--tilemap", help="Zeal Tilemap (ZTM)")

def convert(args):
  tree = ET.parse(args.input)
  root = tree.getroot()
  # layer = root.find("layer")
  data = root.find("layer/data")

  if not data.attrib["encoding"] == 'csv':
    print("Invalid Data Encoding, expected CSV and got ", data.attrib["encoding"])
    return None

  # tiled indexes are 1 based, so we subtract 1 to make it 0 based
  tiles = [255 if int(i) < 1 else int(i) - 1 for i in data.text.replace("\n", "").split(",")]
  return bytes(tiles)

def main():
  args = parser.parse_args()
  print("args", args)
  tilemap = convert(args)

  if not tilemap:
    print("Failed to convert")
    return

  tilemapFileName = args.tilemap
  if tilemapFileName == None:
    tilemapFileName = Path(args.input).with_suffix(".ztm")

  print("tilemap", tilemapFileName) #, tilemap)
  with open(tilemapFileName, "wb") as file:
    file.write(tilemap)


if __name__ == "__main__":
  main()
