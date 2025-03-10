# zlib
zlib is used to decompress assets in GK3. It is also used for compression in libpng.

Grab the latest zlib source code from https://zlib.net/.

## Linux
You can pretty much just use the simple "configure and make" approach here. But one recommendation is to set the install folder to a local subfolder.
```
./configure --prefix=out
make install
```

Copy headers and library files to their final locations:
	- Copy the contents of `out/include` to `Libraries/zlib/linux/include`.
	- Copy the `so` files from `out/lib` to `Libraries/zlib/linux/lib`.
	- DO NOT copy over `share` folder, `pkgconfig` folder, or any static libraries. 

## Mac
The instructions here are virtually identical to the Linux instructions. The main difference is that you'll copy over `dylib` files instead of `so` files (and put them in the `mac` folder of course).

One other thing to highlight: G-Engine currently only supports Intel-based Mac binaries. If you're on an Apple Silicon Mac, you will need to prefix build commands with an `arch` command:
```
arch -arch x86_64 ./configure --prefix=out
arch -arch x86_64 make install
```

Copy headers and library files to their final locations:
	- Copy the contents of `out/include` to `Libraries/zlib/mac/include`.
	- Copy the `dylib` files from `out/lib` to `Libraries/zlib/mac/lib`.
	- DO NOT copy over `share` folder, `pkgconfig` folder, or any static libraries. 

## Windows
Make sure you have Visual Studio installed with C++ Development support. Open a "Visual Studio Developer Command Prompt" - one way to do this from within Visual Studio is to go to `Tools > Command Line > Developer Command Prompt`.

Navigate to the zlib source directory. Build using nmake:
```
nmake /f win32/Makefile.msc
```

This puts the built library in the zlib root folder, mixed amongst all the other files.
	- Copy `zlib.h` and `zconf.h` to `Libraries/zlib/win/include`. 
	- Copy `zdll.lib` and `zlib1.dll` to `Libraries/zlib/win/lib`.
	- DO NOT copy over `zlib.lib` or any other files!

One confusing thing here is that `zlib.lib` is the _static_ version of the library - we don't need it at all! We want to use the dynamic version. To do this, you must link against `zdll.lib` and then include `zlib1.dll` alongside the executable.