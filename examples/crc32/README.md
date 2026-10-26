## CRC32 Checksum

This example shows how to use the CRC32 controller of the Zeal 8-bit VideoBoard to calculate the checksum of given file names.

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

The resulting binary is `build/crc32.bin`. Run it under Zeal 8-bit OS, after transferring it via UART or adding it to a romdisk image.

### Usage

You can generate the CRC32 sum of one or more files thanks to the command:

```
./crc32.bin filename1 <filename2> <filename3> ... <filenameN>
```

### License

This demo is distributed under the CC0-1.0 License.
