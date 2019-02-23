# G-Engine

G-Engine is a C++ game engine that is capable of reading and using the data files from the 3D adventure game *Gabriel Knight 3: Blood of the Sacred, Blood of the Damned* (GK3), developed and published by Sierra Studios in 1998.

The goal of this project is to create a cross-platform game engine that can be used to play GK3. The original game only supported Windows, and it is also a bit buggy on modern hardware. Since GK3 is a heavily data-driven game, the idea is to build a newer, more modern engine that can use the original's data files. Completely new features can also be implemented.

See [my blog post](http://clarkkromenaker.com/post/gengine-01-introduction/) introducing the project for more in-depth info!

## Getting Started

Currently, the project only builds and runs in Xcode as a command line project. Simply download, open in Xcode, and build/run.

Supporting new platforms or build tools should be as easy as creating the necessary build tool files and making sure all required libraries are implemented on those platforms (see below).

### GK3 Data Files
This repository **does not** contain the data files from the game, since those are copyrighted material. You will need to obtain a copy of GK3 (available on Steam or GOG) to get the data files.

The following data files are needed at the moment; copy them into the "Assets" folder before building/running:

- ambient.brn
- common.brn
- core.brn
- day1.brn
- day2.brn
- day3.brn
- day23.brn
- day123.brn

### Libraries

The engine currently uses the following libraries:

- fmod
- GLEW
- minilzo
- SDL
- zlib

All library files are included in the repo, so no software need be installed before running the game.

## Running Tests

This project uses [Catch](https://github.com/catchorg/Catch2) for unit tests.

The Xcode project contains a build target called **GEngine-Tests**. Simply build and run this target to run all defined unit tests.

## Built With

* [SDL](https://www.libsdl.org/) - My training wheels for cross-platform OS polling, rendering, and input
* [fmod](https://www.fmod.com/) - Made SFX and music way easier than I thought possible
* [zlib](https://www.zlib.net/) - For GK3 asset decompression
* [minilzo](http://www.oberhumer.com/opensource/lzo/) - Also for GK3 asset decompression

## Contributing

G-Engine is kind of a pet project of mine, and I'm using it primarily as a learning tool. As such, I am not actively looking for contributions.

That being said, if you are interested in contributing, shoot me an email and we can discuss!

## Authors

* **Clark Kromenaker** - [website](http://clarkkromenaker.com/) - kromenak@gmail.com

## License

This project is licensed under the GNU GPLv3 License - see the [LICENSE.md](LICENSE.md) file for details.

## Acknowledgments

* Sierra On-Line, who inspired me as a kid, and still do today
* [GK3 Tools](https://sourceforge.net/projects/gk3tools/), which provided some guidance on BRN, BSP, and ACT file formats
