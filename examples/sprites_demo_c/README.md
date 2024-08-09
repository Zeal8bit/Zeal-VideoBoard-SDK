## Sprites demo

This demo shows how to display sprites on Zeal 8-bit Video Board:

<!-- [![YouTube video demo](https://img.youtube.com/vi/PeHXfECtwoc/0.jpg)](https://www.youtube.com/shorts/PeHXfECtwoc) -->

### How it works

Zeal 8-bit Video Board supports up to 128 sprites, each sprite is 16x16px, and picks a tile from the 64KB tileset memory (the same as the tilemaps tiles).

This demo uses the 320x240px 8-bit color graphic mode of the video board.
The same demo could be run in 640x480px 8-bit mode too.

### What the program does

The source file, `src/sprites.c`, is meant to be compiled with `sdcc`.

Here are the steps to show and move the sprites, let's omit the core related details (like mapping the VRAM and setting the stack):

* Wait for the V-Blank state, not mandatory but it will prevent screen tearing
* Switch to graphics 320x240 8-bit color mode
* Copy the character palette to the video board palette RAM (physical address `0x100E00`)
* Copy the character tileset to the video board tileset RAM (physical address `0x110000`)
* Fill the entire tilemap layer1 (foreground) with black tiles, even the tiles that are not shown
* Show 2 sprites, stacked
* Wait some time, by waiting a few V-blank states
* Animate the sprites at each interval by changing their X/Y coordinates, swapping their tiles, and updating the flip X attribute.

And that's it!

### Tileset

The original tileset for the character, released under the CC0 license, comes from [this page](https://opengameart.org/content/zelda-like-tilesets-and-sprites).

The character sprites have then been exported using the plugin presented in `tools/` directory at the root of this repository.

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

This demo (`sprites_demo_c/`) is distributed under the CC0-1.0 License.
