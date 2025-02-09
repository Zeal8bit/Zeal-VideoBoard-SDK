
## Filename Flags

Filename flags allow you to provide gif2zeal arguments in the filename of your asset.

This is useful when you have multiple assets, and want to use different settings for
each one while still taking advantage of the ZDE Makefile to auto-export Aseprite/GIF
files to ZTS/ZTP.

Filename flags are provided by adding a trailing `__` (double underscore) to the end
of your filename, then providing the flags as follows.

```text
Flag  Argument
----  --------
Bn    -b N
Cxx   -c N
Pxx   -c N
Sxx   -s N
Z     -z
```

Where `n` is a number, and `xx` is a hex number.  So `S80` will strip 128 tiles, and `B4` will be `-b 4` for 16-color mode

### Example of Filename flags

```text
grid__B1S03.gif
sphere__B8C12S00Z.gif
```

This produces the equivalent of

```python
args Namespace(input='assets/grid__B1S03.gif', tileset=PosixPath('assets/grid.zts'), palette=PosixPath('assets/grid.ztp'), bpp=1, compress=False, colors=256, strip=3)

args Namespace(input='assets/sphere__B8C12S00Z.gif', tileset=PosixPath('assets/sphere.zts'), palette=PosixPath('assets/sphere.ztp'), bpp=8, compress=True, colors=18, strip=0)
```

## Requirements:

Install Pillow

```shell
pip install pillow
```
