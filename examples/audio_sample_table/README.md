## Sound sample table demo

This example shows how to use the sound controller of the Zeal 8-bit VideoBoard to output arbitrary samples. In this example, the binary file `hello_raw_audio.bin` contains raw 16-bit signed audio samples, its sample rate is 11,022Hz. This audio file is a human voice saying "hello", it will be outputted on the audio channel.

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

The resulting binary is `build/audio.bin`. Run it under Zeal 8-bit OS, after transferring it via UART or adding it to a romdisk image.

### Usage

This example program doesn't need any parameter, you can use the following command:

```
./audio.bin
```

If no error occurs, you should hear a human voice saying "hello" on the audio output.

### License

This demo is distributed under the CC0-1.0 License.
