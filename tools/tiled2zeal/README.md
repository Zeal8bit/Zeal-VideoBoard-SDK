## Requirements

* Python


## Usage

This tool currently supports a single layer tilemap in CSV format, with orthogonal orientation in right-down rendering order.

Zeal Video Board supports up to 80x40 tiles per map, with a tilesize of 16x16px

An example Tiled project can be found in (`examples/tilemap/assets/zeal-cave.tiled-project`)

```shell
> ./tiled2zeal.py
usage: gif2zeal [-h] -i INPUT [-m TILEMAP]

> ./tiled2zeal.py -i assets/tilemap.tmx -m assets/tilemap.ztm
args Namespace(input='assets/tilemap.tmx', tilemap='assets/tilemap.ztm')
tilemap assets/tilemap.ztm
```