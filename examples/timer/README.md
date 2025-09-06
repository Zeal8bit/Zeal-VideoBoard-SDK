## Sound testing

This example shows how to use the sound controller of the Zeal 8-bit VideoBoard to output square, triangle and sawtooth waves on a single voice.

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
> The resulting binary is `bin/play.bin`, it can be embedded to a Zeal 8-bit OS romdisk image or transferred via UART to Zeal 8-bit Computer to be executed there.

### Usage

This example program needs a text file containing the notes to play. Then, the file name can be passed as a parameter:

```
./play.bin notes.txt
```

An example `notes.txt` file is provided in the current directory. The syntax of the text file is a follows:
* Comments start with semi-colon character `;`. Any line starting with `;` will be ignored
* The directive `=` is used to specify the duration of the notes in milliseconds, for example `=200` to play all the notes for 200ms each
* The directive `T` is used to specify the type of waveform, 0 for square waves, 1 for triangle waves and 2 for sawtooth. For example: `T2` to play the whole file with sawtooth waves
* `-` is used to specify a pause in the melody
* `[NOTE][OCTAVE][#]` is used to specify the sound to play, **one per line**, where `NOTE` is `DO`, `RE`, `MI`, `FA`, `SOL`, `LA` or`SI`, `OCTAVE` is a number between 1 and 5, and the sharp is optional, and would raise the note by half a tone. For example: `DO2#`, `FA3`, `RE5`, etc...

Keep in mind that the file is parsed **before** playing any sound, as such, it is useless to specify the `T` or `=` directives multiple times in the file, there won't be any dynamic change while playing.

### License

This demo is distributed under the CC0-1.0 License.
