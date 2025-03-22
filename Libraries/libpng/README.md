.# libpng
libpng is used to encode and decode PNG images. This is used for the screenshot tool, as well as saving screenshots for save files.

Grab the latest libpng source code from http://www.libpng.org/pub/png/libpng.html.

## Build Time Configuration
libpng has a lot of features, and we can tune it to only include the features we need for this project. This reduces the file size.

To do this, take `Libraries/libpng/pngusr.dfa` and overwrite the `pngusr.dfa` file in the root of the `libpng` source code.

## Linux
libpng has a dependency on zlib. So, before building, we need to make sure libpng can find zlib headers. The easiest way to do this is to simply copy `zconf.h` and `zlib.h` into the `libpng` source code root folder. Make sure the headers used match the version of zlib that will be used in the final executable!

You can pretty much just use the simple "configure and make" approach here. But one recommendation is to set the install folder to a local subfolder.
```
./configure --prefix=${PWD}/out
make install
```

You can reduce the size of generated library file using the `strip` tool:
```
strip -S libpng16.so.16.47.0
```

You will likely need to change the SONAME baked into the library as well:
```
patchelf -soname libpng.so libpng16.so.16.47.0
```

Copy headers and library files to their final locations:
	- Copy the contents of `out/include/libpng16` to `Libraries/libpng/linux/include`.
	- Copy the `so` files from `out/lib` to `Libraries/libpng/linux/lib`.
	- DO NOT copy over any other folders or files.

## Mac
Before building, make sure libpng can find the zlib headers. The easiest way to do this is to simply copy `zconf.h` and `zlib.h` into the `libpng` source code root folder. Make sure the headers used match the version of zlib that will be used in the final executable!

```
arch -arch x86_64 ./configure --prefix=${PWD}/out
arch -arch x86_64 make install
```

You will likely need to change the generated library's ID so that it loads correctly at runtime:
```
install_name_tool -id @rpath/libpng.dylib out/lib/libpng.dylib
```

Also, you may need to change the zlib loader path as well:
```
install_name_tool -change /usr/lib/libz.1.dylib @rpath/libz.dylib out/lib/libpng.dylib
```

Copy headers and library files to their final locations:
	- Copy the contents of `out/include/libpng16` to `Libraries/libpng/mac/include`.
	- Copy the `dylib` files from `out/lib` to `Libraries/libpng/mac/lib`.
	- DO NOT copy over `share` folder, `pkgconfig` folder, or any static libraries.

## Windows
To build on Windows, it's easiest to use CMake to generate a Visual Studio project, and then build from there. First, generate the CMake project:
```
cmake -S . -B _windows -G "Visual Studio 17 2022" -A Win32 -DZLIB_ROOT=~/Projects/gengine/Libraries/zlib/win/
```
Since libpng depends on zlib, we need to specify the zlib root in this command. As you can see, it's possible to just point it to the `Libraries/zlib/win` folder!

Once the solution is generated, open it. Change the configuration to "Release" in the top bar. Right-click on `png_shared` and build it.

Finally, copy the library over to `Libraries/libpng/win`:
	- From the libpng source code root, copy `png.h`, `pngconf.h`, and `pnglibconf.h` to `Libraries/libpng/win/include`.
	- From the `_windows/Release` folder, copy `libpng16.dll` and `libpng16.lib` to `Libraries/libpng/win/lib`.