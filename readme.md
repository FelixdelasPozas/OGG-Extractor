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

## Options
The tool can be configured to show only files with a minimum size making easier to identify music files from smaller dialog sound files. 

# Compilation requirements
## To build the tool:
* cross-platform build system: [CMake](http://www.cmake.org/cmake/resources/software.html).
* compiler: [Mingw64](http://sourceforge.net/projects/mingw-w64/) on Windows or [gcc](http://gcc.gnu.org/) on Linux.

## External dependencies:
The following libraries are required:
* [Qt opensource framework](http://www.qt.io/).

# Install
The only current option is build from source as binaries are not provided. 

# Screenshots
Simple main dialog.

![Main dialog](https://cloud.githubusercontent.com/assets/12167134/7867872/e2fd4c28-0578-11e5-93bb-56c7ee8b26df.jpg)

Dialog shown while scanning for files.

![Process Dialog](https://cloud.githubusercontent.com/assets/12167134/7867873/e48c0714-0578-11e5-8de4-ba1b44b1b72f.jpg)

# Repository information
**Version**: 1.4.0

**Status**: finished

**cloc statistics**

| Language                     |files          |blank        |comment           |code  |
|:-----------------------------|--------------:|------------:|-----------------:|-----:|
| C++                          |   11          |  584        |    374           |2362  |
| C/C++ Header                 |   10          |  242        |    631           | 448  |
| CMake                        |    1          |   19        |     11           |  90  |
| **Total**                    |   **22**      |  **845**    |   **1016**       |**2900**|
