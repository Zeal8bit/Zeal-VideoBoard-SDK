# Zeal 8-bit Video Board Software Development Kit (SDK)

![Zeal video board](img/videoboard.png)

This repository contains everything you need to start developing for the Zeal 8-bit Video Board v1.0.0 and above.

<p align="center">
    <a href="https://www.tindie.com/products/zeal8bit/zeal-8-bit-video-board-homebrew-vga-board/">Click here to get your own video board,</a> or <a href="https://zeal8bit.com/getting-started-zvb/">here to check the specification</a>!
<p>

## Table of Contents
- [Zeal 8-bit Video Board Software Development Kit (SDK)](#zeal-8-bit-video-board-software-development-kit-sdk)
  - [Table of Contents](#table-of-contents)
  - [Software side](#software-side)
    - [Installation](#installation)
    - [Usage](#usage)
    - [API Reference](#api-reference)
    - [Code Examples](#code-examples)
    - [SDK Changelog](#sdk-changelog)
  - [Hardware side](#hardware-side)
    - [Hardware documentation](#hardware-documentation)
    - [Hardware Changelog](#hardware-changelog)
    - [Firmware Changelog](#firmware-changelog)
  - [Contributing](#contributing)
  - [License](#license)
  - [Contact information](#contact-information)


## Software side

### Installation

This repository comes with a pre-built library of the SDK located in `lib/` directory. This file is meant to be linked with programs written in C and compiled with [SDCC](https://sdcc.sourceforge.net) compiler.

The demos written in Z80 assembler, as well as the assembly "header" files, are meant to be assembled with z88dk's z80asm assembler. Check their [official Github page](https://github.com/z88dk/z88dk) for more information about how to install it.

To compile the examples, you will need to define the path to this current repository in the environment variable `ZVB_SDK_PATH`:

```
export ZVB_SDK_PATH=/path/to/Zeal-VideoBoard-SDK
```

### Usage

The fastest way to test this library is to define the `ZVB_SDK_PATH` environment variable, and compile one of the example in `examples/` directory.

For the examples that are meant to be run on Zeal 8-bit OS, you will also need to define `ZOS_PATH` environment which should point to Zeal 8-bit OS source code directory.

If you plan on making your own program, you will first need to choose the language, C or Z80 assembly, with these differences:

* Using C, and SDCC compiler, you are able to use the pre-built library which contains several functions to setup the video mode, the tileset, the tilemap, etc.... It is also possible to directly communicate and write the video board registers thanks to the C header files from `include/`. **Please note that your program must run on Zeal 8-bit OS**
* Using Z80 assembly, and z88dk's z80asm assembler, you can write a program that can run either in Zeal 8-bit OS, or in a bare metal environment. There is currently no library or helper implemented for assembly, but there is a header file that documents and defines all the constants: [include/zvb_hardware_h.asm](include/zvb_hardware_h.asm)
If you plan on running your program on Zeal 8-bit Computer, you may need to use the MMU to map and unmap the video memory, check [the open source MMU header](https://github.com/Zeal8bit/Zeal-8-bit-OS/blob/main/target/zeal8bit/include/mmu_h.asm)

### API Reference

At the moment, this API reference is only referring to the C library as there is no assembly implementation.

* Graphics: This part of the library defines functions to control the screen, set the color palettes, manipulate the tilesets and tilemaps. These functions are declared and documented in [`include/zvb_gfx.h`](include/zvb_gfx.h) header file.
* Controller: TBD, library to manage input devices such as game controllers or joysticks.
* Audio: TBD, library to manage the sound output.

### Code Examples

This repository presents graphic demos, written in assembly that can be run directly on Zeal 8-bit Computer without any OS, after compiling them, it is possible torun them directly from Zeal 8-bit Bootloader, via the `Load program from UART` option in the menu.

It is also possible to flash the binary directly to the NOR Flash, but this is more tedious and requires an external flasher.

For the snake game, the resulting `bin/snake.bin` file can then be transferred to Zeal 8-bit OS (or imported in the romdisk) and run via the command `./snake.bin`.

Please refer to the `README.md` file that are present in each example directory.

### SDK Changelog

For the revision of the SDK itself, please refer to [Releases page](https://github.com/Zeal8bit/Zeal-VideoBoard-SDK/releases)



## Hardware side

### Hardware documentation

TBD: The hardware documentation is not available yet.

### Hardware Changelog

Here is the changelog for the hardware design of Zeal 8-bit Video Board:

* **v1.0.0**: first public release of Zeal 8-bit Video Board hardware, available since April 2024.

### Firmware Changelog

Here is the changelog for the Zeal 8-bit Video Board firmware (bitstream)

* **v0.1.0**: first public release shipped with the version `v1.0.0` of the hardware.


## Contributing

Contributions are welcome! Feel free to fix any bug that you may see or encounter, or implement any feature that you find important.

To contribute:

* Fork the Project
* Create your feature Branch (optional)
* Commit your changes. Please make a clear and concise commit message.
* Push to the branch
* Open a Pull Request

An example of a good commit message is as follows:

```
Module: add/fix/remove a from b

Explanation on what/how/why
```

For example:

```
GFX: add support for bitmap compressed tileset

It is now possible to load tilesets that are compressed as 2-color bitmaps.
```

## License

The source code for the library, its implementation, and the tools are distributed under the Apache 2.0 License. See LICENSE file for more information.

You are free to use it for personal and commercial use, the boilerplate present in each file must not be removed.

The examples and demos are distributed under the CC0-1.0 License.

Check the header of each file to see which License it is distributed under.

## Contact information

For any suggestion or request, you can contact me at contact [at] zeal8bit [dot] com

For feature requests, you can also open an issue or a pull request.
