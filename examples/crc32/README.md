## CRC32 Checksum

This example shows how to use the CRC32 controller of the Zeal 8-bit VideoBoard to calculate the checksum of given file names.

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
> The resulting binary is `bin/crc32.bin`, it can be embedded to a Zeal 8-bit OS romdisk image or transferred via UART to Zeal 8-bit Computer to be executed there.

### Usage

You can generate the CRC32 sum of one or more files thanks to the command:

```
./crc32.bin filename1 <filename2> <filename3> ... <filenameN>
```

### License

This demo is distributed under the CC0-1.0 License.
