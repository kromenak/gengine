# ffmpeg
ffmpeg is used to play video files in G-Engine. This README explains how to update ffmpeg for all platforms.

Grab the latest source code from https://www.ffmpeg.org/download.html#releases.

## Linux
Copy `build.sh` to the root of the ffmpeg source code folder. Run the script from the command line:
```sh
./build.sh
```
The library is built to the `out` directory. Copy the `include` and `lib` folders to `Libraries/ffmpeg/linux`.

## Mac
Make sure nasm is installed:
```sh
brew install nasm
```

Then, copy `build.sh` to the root of the ffmpeg source code folder. You can then run this script from the terminal:
```sh
./build.sh
```
The library is built to the `out` directory. Copy the `include` and `lib` directories to `Libraries/ffmpeg/mac`.

## Windows
ffmpeg can be built using Visual Studio, but it must be built through a unix-like shell. Be sure the following are installed:
- Visual Studio 2022+ with C++ Development support
- MSYS2

Open a "Visual Studio Developer Command Prompt" - one way to do this from within Visual Studio is to go to `Tools > Command Line > Developer Command Prompt`.

Navigate to the MSYS2 install directory (probably `C:\msys64`) and run this command:
```cmd
msys2_shell.cmd -use-full-path
```
This opens an MSYS2 shell that inherits the environment variables of the Developer Command Prompt. In other words, you can access Visual Studio build tools inside of MSYS2 shell!

Within MSYS2, install nasm.
```sh
pacman -S nasm
```

Then, copy `build.sh` to the root of the ffmpeg source code folder. You can then run this script from the msys2 shell:
```sh
./build.sh
```
The library is built to the `out` directory. Copy the `include` and `bin` directories to `Libraries/ffmpeg/win`. Note that you will also likely need to update `CMakeLists.txt`, since the names of the DLLs include a version number that usually changes between releases.