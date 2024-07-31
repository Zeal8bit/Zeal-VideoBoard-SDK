## Sound sample table demo

This example shows how to use the sound controller of the Zeal 8-bit VideoBoard to output arbitrary samples. In this example, the binary file `hello_raw_audio.bin` contains raw 16-bit signed audio samples, its sample rate is 11,022Hz. This audio file is a human voice saying "hello", it will be outputted on the audio channel.

### Compiling

To compile the demo, you will need both the Zeal 8-bit OS headers and the compiled Zeal 8-bit Video Board SDK, then make sure you defined both environment variables:
```
export ZVB_SDK_PATH=/path/to/zeal-svb-sdk
export ZOS_PATH=/path/to/zeal-8bit-os
```

After defining both, you can simply use:

```
make
```

Keep in mind that you will need `sdcc` v4.2.0 or newer to compile the program.

> [!NOTE]
> The resulting binary is `bin/audio.bin`, it can be embedded to a Zeal 8-bit OS romdisk image or transferred via UART to Zeal 8-bit Computer to be executed there.

### Usage

This example program doesn't need any parameter, you can use the following command:

```
./audio.bin
```

If no error occurs, you should hear a human voice saying "hello" on the audio output.

### License

This demo is distributed under the CC0-1.0 License.
