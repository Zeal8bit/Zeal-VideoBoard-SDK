## Tilemap Demo

This demo shows how to display Tilemaps on Zeal 8-bit Video Board:

<!-- [![YouTube video demo](https://img.youtube.com/vi/PeHXfECtwoc/0.jpg)](https://www.youtube.com/shorts/PeHXfECtwoc) -->

### How it works

Zeal 8-bit Video Board supports up to 256 tiles (512 tiles in 4-bit mode), each tile is 16x16px.

This demo uses the 320x240px 8-bit color graphic mode of the video board.

### What the program does

The source file, `src/tilemap.c`, is meant to be compiled with `sdcc`.

Here are the steps to show the tilemap, let's omit the core related details (like moving the sprite, initializing vram, etc):

* Disable the screen, to prevent screen tearing during initialization
* Switch to graphics 320x240 8-bit color mode
* Copy the tilemap palette to the video board palette RAM (physical address `0x100E00`)
* Copy the tilemap tileset to the video board tileset RAM (physical address `0x110000`)
* Load the tilemap

And that's it!

### Tileset

The original tileset for the tilemap, released under the [Public Domain](http://creativecommons.org/publicdomain/zero/1.0/) license, comes from [OpenGameArt.org](https://opengameart.org/content/spelunky-inspired-tileset).

The tileset graphics have been exported to GIF using Aseprite. The exported GIF was then converted to ZTS/ZTP format with tools found in (`tools/zeal2gif/gif2zeal.py`).  It was then converted back to GIF with (`tools/zeal2gif/zeal2gif.py`) for use in _Tiled_.

The tilemap was built using [Tiled](https://www.mapeditor.org/), and the TMX file was processed with the tools in (`tools/tiled2zeal/tiled2zeal.py`), producing the ZTM file.

### Compiling

To compile the demo, you will need the Zeal 8-bit Video Board SDK header file, make sure you define the following environment variable:
```
export ZVB_SDK_PATH=/path/to/zeal-svb-sdk
```

After defining it, you can simply use:

```
make
```

### License

This demo (`tilemap/`) is distributed under the CC0-1.0 License.