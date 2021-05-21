MINIGSF: Numbered minigsf creation utility
==========================================
[![Travis Build Status](https://travis-ci.org/loveemu/minigsf.svg?branch=master)](https://travis-ci.org/loveemu/minigsf) [![AppVeyor Build Status](https://ci.appveyor.com/api/projects/status/2cd81xqyxb37pp9g/branch/master?svg=true)](https://ci.appveyor.com/project/loveemu/minigsf/branch/master)

Numbered minigsf creation utility, where the offset is known.

Downloads
---------

- [Latest release](https://github.com/loveemu/minigsf/releases/latest)

Usage
-----

Syntax: `minigsf [Base name] [Offset] [Size] (Count)`

or

Syntax: `minigsf [Base name] [Offset] =[Hex pattern] (Count)`

### Options

`--help`
  : Show help

`--psfby`, `--gsfby` [name]
  : Set creator name of GSF

`-o` [basename]
  : Specify minigsf filename (without extension)

### Examples

```bash
minigsf AGB-SMPL-USA 0x80cafe8 1 64
```

```bash
minigsf AGB-SMPL-USA 0x80cafe8 =NNNNNNNNDEADBEEF 64
```

```bash
minigsf -o songtitle AGB-SMPL-USA 0x80cafe8 =00FACE08
```
