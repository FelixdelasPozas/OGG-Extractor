Ogg Extractor
=============

# Summary
- [Description](#description)
- [Compilation](#compilation-requirements)
- [Install](#install)
- [Command-line version](#command-line-version)
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

# Command-line version
A command-line version of the application is included as an executable ending in "-cli". The filtering options are the same, 
but the CLI version won't let you listen to or rename the found OGG streams before extracting. The program arguments for the
command-line version are:

| Option                       | Description   |
|:-----------------------------|:--------------|
| **-h**                       | Shows help.   |
| **-s \<number\>**              | Filter found streams by size (in Kb). |
| **-l \<number\>**              | Filter found streams by length in seconds. |
| **-o \<output_dir\>**          | Specify output directory for files. |
| **-i \<input_file\>**          | Specify input file to scan for OGG streams. |
| **-d**                       | Do not extract OGG streams, just dump stream information in a CSV file. |
| **-r \<range\>**               | Ranges or positions to extract separated by commas. | 

Ranges are specified as lower_pos-upper_pos and both positions are included. For example '1,3,7-10' will 
extract the OGG streams in the positions 1,3,7,8,9 and 10. Positions start at 1.
                  
The CLI version do not depend on Qt library and the only file needed for it to work is the executable. It's main 
purpose is to be used inside command-line scripts to scan and extract files in bulk but of course can be used as-is.
Take into consideration the filtering by size or length do not affect track number and that the combination of filters
and range can produce no files being extracted. 

# Screenshots
Simple main dialog.

![Main dialog](https://user-images.githubusercontent.com/12167134/55731451-8a73b880-5a1a-11e9-9343-712513c41f1d.jpg)

Command-line version

![CLI version](https://user-images.githubusercontent.com/12167134/261821158-19554381-b7c6-4d94-8353-095ccf02b0bc.png)

# Repository information
**Version**: 1.8.0

**Status**: finished

**License**: GNU General Public License 3

**cloc statistics**

| Language                     |files          |blank        |comment           |code  |
|:-----------------------------|--------------:|------------:|-----------------:|-----:|
| C++                          |    6          |  341        |    196           | 1253 |
| C/C++ Header                 |    4          |   87        |    256           | 149  |
| CMake                        |    1          |   21        |      9           |  93  |
| **Total**                    |   **11**      |  **449**    |   **461**        |**1495**|
