# Planet X3 MIDI converter

A simple utility which converts a Standard MIDI File (.MID) to a format which Planet X3 supports (it is just a long MIDI event dump).

**IMPORTANT**: The files that this program outputs are uncompressed! They will *not* load in a standard Planet X3 installation, and will likely result in the game crashing upon attempting to load!

## Dependencies

- a *NIX system (only GNU/Linux has been tested so far), Windows Subsystem for Linux also theoretically possible
- Python 3
- [midicsv](https://www.fourmilab.ch/webtools/midicsv/) (usually available directly from your system's repo)
- libfluidsynth (optional, only required for the player)

## Usage

To convert a MIDI file, run the following:

```
./convert.sh <input file> <output file> <header byte>
```

Each music file for Planet X3 has to contain a specific header byte, which tells the engine what device to play the file on and which decoder to use. For MIDI, only the following values are allowed:

- `5` → for Roland SC-55 (and other GM-compatible synths)
- `6` → for Roland MT-32 (and compatibles)

Other values will result in undefined behavior.

So, for example, if we want to convert the main menu song for the MT-32, we run the following:

```
./convert.sh ~/Documents/Planet\ X3\ Digital\ Download/Soundtrack/Roland\ MT-32/MIDIFILES/PX3\ 15\ Into\ Battle\ \(MT-32\).mid menu.bin 6
```

Replace the path to wherever your Planet X3 Digital Download is actually located on your system.

By running this command, the original MIDI file will be converted to `midi.bin`, which should appear in the same directory as this program.

## Player

This repository also contains a small player program based on FluidSynth.

**NOTE**: This player is only intended to test the functionality of the output. While files intended for SC-55 will sound flawless, the quality will severely decrease when attempting to play files intended for the MT-32, as that variant of the soundtrack requires custom instruments to be loaded, which is not possible here.

To compile it, run:

```
cc playbin.c -lfluidsynth playbin
```

This program also requires a soundfont file (`gm.sf2`) to be placed in the same directory. There are several free soundfonts available, we recommend the [Fluid R3 GM SoundFont](https://github.com/urish/cinto/blob/master/media/FluidR3%20GM.sf2), as it is one of the best free soundfonts available. Just download it, move it to the same directory as this program and rename it to `gm.sf2`.

To play a converted file (after you have supplied your own soundfont file), run:

```
./playbin manu.bin
```
