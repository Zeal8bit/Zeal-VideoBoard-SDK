## Flag example

This example shows how to create and show the flag of France on screen thanks to color tiles, generated at runtime.

![Flag screenshot](img/screen.png)

### Presentation

This example is implemented in C. This was made possible thanks to [Zeal 8-bit OS](https://github.com/Zeal8bit/Zeal-8-bit-OS), which supports C programs compiled with SDCC. As such, this example **cannot** be executed as a raw/standalone binary, it needs to be executed within Zeal 8-bit OS.

The Zeal 8-bit Video Board (ZVB) compiled library is necessary to compile this example, please refer to the [README.md](../../README.md) at the root of this repository.

The example runs in the 8-bit colors 320x240px graphics mode, the foreground layer is not used and simply contains transparent tiles, the background will display the colored tiles. It doesn't use any sprite.

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
> The resulting binary that contains the game is `bin/flag.bin`. This binary can be embedded to a Zeal 8-bit OS romdisk image or transferred via UART to Zeal 8-bit Computer to be executed there.

### License

This example (`flag/`) is distributed under the CC0-1.0 License.
