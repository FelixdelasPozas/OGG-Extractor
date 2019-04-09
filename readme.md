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
* 'The Chronicles Of Riddick: Escape From Butcher Bay' by Starbreeze Studios.
* 'The Chronicles Of Riddick: Assault On Dark Athena' by Starbreeze Studios.

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
Download and execute the latest installer from the [releases](https://github.com/FelixdelasPozas/OGG-Extractor/releases) page. 

# Screenshots
Simple main dialog.

![Main dialog](https://user-images.githubusercontent.com/12167134/55731451-8a73b880-5a1a-11e9-9343-712513c41f1d.jpg)

# Repository information
**Version**: 1.7.0

**Status**: finished

**License**: GNU General Public License 3

**cloc statistics**

| Language                     |files          |blank        |comment           |code  |
|:-----------------------------|--------------:|------------:|-----------------:|-----:|
| C++                          |    5          |  253        |    135           | 849  |
| C/C++ Header                 |    4          |   85        |    248           | 147  |
| CMake                        |    1          |   18        |     12           |  64  |
| **Total**                    |   **10**      |  **356**    |   **395**        |**1060**|
