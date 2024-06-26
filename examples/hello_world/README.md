## Hello, world!

This demo is small demo that shows how to display a colored messaged on Zeal 8-bit Video Board:

![Hello world text](img/text.png)

### How it works

Zeal 8-bit Video Board supports text mode, which can be accessed either via the tilemap (memory mapped) or via the text controller (I/O mapped).

This example uses the text controller to print a hello world message, while switching the foreground color for each character.

### Compiling

To compile the demo, simply use:

```
make
```

Keep in mind that you will need `z88dk`'s `z80asm` to assemble the program.

### License

This demo (`hello_world/`) is distributed under the CC0-1.0 License.
