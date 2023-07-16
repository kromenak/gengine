# G-Engine
G-Engine is a C++ game engine that can parse and use the data files from the classic 3D adventure game *Gabriel Knight 3: Blood of the Sacred, Blood of the Damned* (GK3), developed and published by Sierra Studios in 1999.

The goal of this project is to create a cross-platform game engine capable of playing GK3. The original game only supported Windows, and it's also a bit buggy on modern hardware. Since GK3 is heavily data-driven, the idea is to build a newer, more modern engine that can use the original's data files. Completely new features can also be implemented!

See [my blog post](http://clarkkromenaker.com/post/gengine-01-introduction/) introducing the project for more in-depth info!

## Getting Started
Currently, GEngine runs on Mac or Windows. Here's how to get up and running.

### GK3 Data Files
This repository **does not** contain the game's data files, since those are copyrighted material. You will need to obtain a copy of GK3 (available on Steam or GOG) to get the data files. The data files are all Barn (.brn) asset bundles and all video files (.bik and .avi) in the game's `Data` directory.

If building from source, put all data files into the `Data` directory before building/running.

Prebuilt executables (such as those obtained on the "Releases" page) also require data files to be copied before running. On Windows, copy them to the `Data` folder in the exe directory. On Mac, copy them to `Gabriel Knight 3.app/Contents/Resources/Data`.

The [Gabriel Knight 3 demo](https://archive.org/details/GabrielKnight3BloodOfTheSacredBloodOfTheDamnedDemo) is also supported. Simply copy `Gk3demo.brn` into the data folder. The engine will detect this file and launch in "demo mode". This is a way you can test out the engine without purchasing the full game.

### Build File Generation
The project uses CMake, which means the definitions of what targets to create, which source files to include, and which libraries to link against are defined in `CMakeLists.txt` in the project root. Using CMake, you can generate the project files for your preferred IDE.

I've only tested Xcode and Visual Studio at this point, but it'll likely work with other build systems with a bit of effort.

#### Mac (Xcode)
Make sure Xcode and CMake are installed. Then, use terminal to execute the script at `Build/GenerateMac.sh`. The Xcode project will be generated at `Build/Mac`. Build and run the `gk3` target.

#### Windows (Visual Studio 2022)
Install Visual Studio 2022 and CMake. When you install CMake, I'd recommend adding it to your path.

Then, I'd recommend using [Git Bash](https://gitforwindows.org/) to execute the `Build/GenerateWindows.sh` script. This will generate a VS 2022 project at `Build/Windows`. Build and run the `gk3` target.

If you'd prefer to use the native Command Prompt, you'll just need to manually run the CMake command in the bash script file instead.

## Platform Support
The engine currently supports Mac and Windows. Most of the engine code is platform-agnostic, but there are some key things that must be taken care of to support a new platform.

### Libraries
The engine currently uses the following third-party libraries:

- ffmpeg
- fmod
- GLEW
- minilzo
- SDL
- stb
- zlib

All library files are included in the repo, so no software need be installed before building and running. To support a new platform, the library files for that platform must be built and added to the appropriate `Libraries` subdirectory.

### CMake
Most of the CMake files probably don't need to change to support a new platform, but there are some platform-specific bits in there. To support a new platform, the CMake file likely needs to be modified.

### Platform-Specific Code
A lot of platform-specific code is taken care of by SDL. That being said, because the engine relies on SDL, you can only build and run on a platform that is supported by SDL!

There are also some platform-specific File and System functions that may need to be implemented for a new platform. This includes functionality for loading files from the disk, retrieving the computer name, etc. 

## Running Tests
This project uses [Catch2](https://github.com/catchorg/Catch2) for unit tests.

After generating build files with CMake, simply run the `tests` target to run tests.

## Built With

* [SDL](https://www.libsdl.org/) - My training wheels for cross-platform OS polling, rendering, and input
* [ffmpeg](https://ffmpeg.org/) - Provides AVI and Bink video support. Without this library, I have no idea how I'd ever get video playback working
* [fmod](https://www.fmod.com/) - Made SFX and music way easier than I thought possible
* [zlib](https://www.zlib.net/) - For GK3 asset decompression
* [minilzo](http://www.oberhumer.com/opensource/lzo/) - Also for GK3 asset decompression
* [stb](https://github.com/nothings/stb) - Handy public domain utilities; I'm currently using the image resize algorithms

## Contributing
G-Engine is kind of a pet project of mine, and I'm using it primarily as a learning tool. As such, I am not actively looking for contributions.

If you are interested in contributing, you can submit a pull request. However, I request that you do not make pull requests that massively refactor or change how things are implemented. Shoot me an email or open an issue so we can discuss/debate such changes beforehand.

## Progress
You may be wondering: what's the status of this project? Is the game playable? Do the graphics work? Here's a screenshot of what the game currently looks like:

![Screenshot](Media/Screenshot-0.png?raw=true "Screenshot")

(See the "Media" folder for a few more screenshots).

Graphically, the game looks pretty close to the original. You can't tell from the screenshots, but 3D model and face animations are working. A few things that are still missing: lighting on non-BSP models, correct texture filtering, billboard effect on 3D trees, graphical artifacts here and there, etc.

Logically, the game runs, but is definitely not completable. You can walk around, interact with objects, move between rooms, and even talk to NPCs. However, a lot of important internal functions the game relies on to function are not yet implemented, which will likely result in softlocks or crashes in many cases.

## Running the Game
From time to time, as new and interesting features are added to the game, I'll package and upload releases here on GitHub. These executables are not "signed" or "notarized", so your OS may complain about files being from an unknown developer. You'll have to use the OS mechanism to bypass such warnings if you want to run the game.

Remember to copy the contents of GK3's "Data" folder to the "Data" folder. If running on Mac, this folder is inside of the .app package - you'll need to right-click on "Gabriel Knight 3.app", select "Show Package Contents", and navigate to the "Content/Resources" folder. If you don't do this, the game will provide an error message about missing "brn" files and fail to run.

As mentioned above, some functionality is implemented, but expect bugs, crashes, poor performance, and missing/incorrect behavior. Do not expect to play the entire game, or even complete a single timeblock!

You can open the developer console using the `~` key, which allows you to enter commands in the Sheep language. For example, try `SetLocation("LBY")` or `PrintString("Hello World")`. Check out the GitHub wiki for a list of all Sheep API functions that can be called.

F1, F2, and F3 enable debug visualizations. F4 quits the game.

## Authors

* **Clark Kromenaker** - [website](http://clarkkromenaker.com/) - kromenak@gmail.com

## License
This project is licensed under the GNU GPLv3 License - see the [LICENSE.md](LICENSE.md) file for details.

## Acknowledgments/Thanks
* Sierra On-Line: they inspired me to get into game development long ago, and they still inspire me today
* Jane Jensen: for creating and designing such a compelling video game series
* The GK3 Team: for creating a super fun game, and including a wealth of docs hidden in the data files
* [GK3 Tools](https://sourceforge.net/projects/gk3tools/): for providing guidance on reverse engineering BRN, BSP, and ACT file formats
