## Hello, world!

This demo is small demo that shows how to display a colored messaged on Zeal 8-bit Video Board:

![Hello world text](img/text.png)

### How it works

Zeal 8-bit Video Board supports text mode, which can be accessed either via the tilemap (memory mapped) or via the text controller (I/O mapped).

This example uses the text controller to print a hello world message, while switching the foreground color for each character.

### Compiling

From this example directory, build with [Zeal Development Environment (ZDE)](https://github.com/zoul0813/zeal-dev-environment):

```sh
zde cmake
```

ZDE provides the toolchain and SDK environment. The output is placed in `build/`.

Alternatively, build directly with CMake using the following dependencies and environment variables.

Use CMake 3.16 or newer and `z88dk-z80asm`. The Zeal 8-bit OS checkout supplies the z88dk CMake toolchain; these demos run directly on the hardware without an OS.

```sh
export ZVB_SDK_PATH=/path/to/Zeal-VideoBoard-SDK
export ZOS_PATH=/path/to/Zeal-8-bit-OS
cmake -S . -B build
cmake --build build
```

The resulting standalone binary is `build/hello.bin`.

### License

This demo (`hello_world/`) is distributed under the CC0-1.0 License.
