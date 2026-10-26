## Sound testing

This example shows how to use the sound controller of the Zeal 8-bit VideoBoard to output square, triangle and sawtooth waves on a single voice.

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

The resulting binary is `build/play.bin`. Run it under Zeal 8-bit OS, after transferring it via UART or adding it to a romdisk image.

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
