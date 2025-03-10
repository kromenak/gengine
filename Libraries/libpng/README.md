# libpng
libpng is used to encode and decode PNG images. This is used for the screenshot tool, as well as saving screenshots for save files.

Grab the latest libpng source code from http://www.libpng.org/pub/png/libpng.html.

## Build Time Configuration
libpng has a lot of features, and we can tune it to only include the features we need for this project. This reduces the file size.

To do this, take `Libraries/libpng/pngusr.dfa` and overwrite the `pngusr.dfa` file in the root of the `libpng` source code.

## Linux
libpng has a dependency on zlib. So, before building, we need to make sure libpng can find zlib headers. The easiest way to do this is to simply copy `zconf.h` and `zlib.h` into the `libpng` source code root folder. Make sure the headers used match the version of zlib that will be used in the final executable!

You can pretty much just use the simple "configure and make" approach here. But one recommendation is to set the install folder to a local subfolder.
```
./configure --prefix=out
make install
```

This puts the built library in the `out` subfolder.
- Replace the contents of `zlib/linux/include` with the contents from `out/include` (`zconf.h` and `zlib.h`).
- Replace the contents of `zlib/linux/lib` with the `so` files in `out/lib`.
- DO NOT copy over any other files (`a` files, `pkgconfig` files, `share` folder, etc).

## Mac
The instructions here are virtually identical to the Linux instructions. The main difference is that you'll copy over `dylib` files instead of `so` files (and put them in the `mac` folder of course).

One other thing to highlight: G-Engine currently only supports Intel-based Mac binaries. If you're on an Apple Silicon Mac, you will need to prefix build commands with an `arch` command:
```
arch -arch x86_64 ./configure --prefix=out
arch -arch x86_64 make install
```

## Windows
To build on Windows, it's easiest to use CMake to generate a Visual Studio project, and then build from there.

First, generate the CMake project:
```
cmake -S . -B _windows -G "Visual Studio 17 2022" -A Win32 -DZLIB_ROOT=~/Projects/gengine/Libraries/zlib/win/
```
Since libpng depends on zlib, we need to specify the zlib root in this command. As you can see, it's possible to just point it to the `Libraries/zlib/win` folder!

Once the solution is generated, open it. Change the configuration to "Release" in the top bar. Right-click on `png_shared` and build it.

Finally, copy the library over to `Libraries/libpng/win`:
	- From the libpng source code root, copy `png.h`, `pngconf.h`, and `pnglibconf.h` to `Libraries/libpng/win/include`.
	- From the `_windows/Release` folder, copy `libpng16.dll` and `libpng16.lib` to `Libraries/libpng/win/lib`.