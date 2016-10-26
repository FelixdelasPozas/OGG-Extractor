Ogg Extractor
=============

# Summary
- [Description](#description)
- [Compilation](#compilation-requirements)
- [Install](#install)
- [Screenshots](#screenshots)
- [Repository information](#repository-information)

# Description
Tool to scan and extract OGG sound files from big game data files. This program has been used to extract the music from the following games:
* 'The Talos Principle' by Croteam.
* 'Thief: Deadly Shadows' by Ion Storm.
* 'Hitman 2: Silent Assassin' by Io Interactive.
* 'Hitman: Contracts' by Io Interactive.
* 'Freedom Fighters' by Io Interactive. 
* 'Blood Omen 2' by Crystal Dynamics. 
* 'FEZ' by Polytron Corporation.

It will probably work with any program that uses unencrypted OGG Vorbis audio files. 

## Options
The tool can be configured to show only files with a minimum size or duration, making it easier to differentiate music files from dialog ones. 
The found OGG files can be renamed and played before the extraction process so it can be selected or deselected to be extracted. 

# Compilation requirements
## To build the tool:
* cross-platform build system: [CMake](http://www.cmake.org/cmake/resources/software.html).
* compiler: [Mingw64](http://sourceforge.net/projects/mingw-w64/) on Windows or [gcc](http://gcc.gnu.org/) on Linux.

## External dependencies:
The following libraries are required:
* [Qt opensource framework](http://www.qt.io/).
* [Xiph's vorbis and ogg libraries](https://www.xiph.org/).

# Install
The only current option is build from source as binaries are not provided. 

# Screenshots
Simple main dialog.

![Main dialog](https://cloud.githubusercontent.com/assets/12167134/15631646/14edcc32-2574-11e6-8aff-13938efe0811.jpg)

# Repository information
**Version**: 1.5.0

**Status**: finished

**cloc statistics**

| Language                     |files          |blank        |comment           |code  |
|:-----------------------------|--------------:|------------:|-----------------:|-----:|
| C++                          |    4          |  218        |    108           | 735  |
| C/C++ Header                 |    3          |   62        |    185           |  99  |
| CMake                        |    1          |   18        |     12           |  59  |
| **Total**                    |   **8**      |  **298**    |   **305**       |**893**|
