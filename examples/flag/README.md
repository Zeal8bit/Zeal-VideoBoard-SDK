## Flag example

This example shows how to create and show the flag of France on screen thanks to color tiles, generated at runtime.

![Flag screenshot](img/screen.png)

### Presentation

This example is implemented in C. This was made possible thanks to [Zeal 8-bit OS](https://github.com/Zeal8bit/Zeal-8-bit-OS), which supports C programs compiled with SDCC. As such, this example **cannot** be executed as a raw/standalone binary, it needs to be executed within Zeal 8-bit OS.

The Zeal 8-bit Video Board (ZVB) compiled library is necessary to compile this example, please refer to the [README.md](../../README.md) at the root of this repository.

The example runs in the 8-bit colors 320x240px graphics mode, the foreground layer is not used and simply contains transparent tiles, the background will display the colored tiles. It doesn't use any sprite.

### Compiling

From this example directory, build with [Zeal Development Environment (ZDE)](https://github.com/zoul0813/zeal-dev-environment):

```sh
zde cmake
```

ZDE provides the toolchain and SDK environment. The output is placed in `build/`.

Alternatively, build directly with CMake using the following dependencies and environment variables.

Use CMake 3.16 or newer, SDCC 4.2.0 or newer, the Zeal 8-bit OS SDK (including its CMake toolchain and compiled SDCC runtime), and the compiled Zeal Video Board SDK:

```sh
export ZVB_SDK_PATH=/path/to/Zeal-VideoBoard-SDK
export ZOS_PATH=/path/to/Zeal-8-bit-OS
cmake -S . -B build
cmake --build build
```

The resulting binary is `build/flag.bin`. Run it under Zeal 8-bit OS, after transferring it via UART or adding it to a romdisk image.

### License

This example (`flag/`) is distributed under the CC0-1.0 License.
