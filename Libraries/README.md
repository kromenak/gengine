# Libraries
Each subfolder here is a third-party library used by G-Engine.

## Types of Libraries
Third party libraries come in a few different flavors:

- The full source code for a library. This code is compiled at the same time the engine is compiled, and gets embedded directly in the final executable. Examples include Flex, imgui, minilzo, and stb.

- A static library. This code is already compiled, and it is then linked into the final executable. All code from the library gets embedded directly in the final executable. There are not yet any of these in the project!

- A dynamic library. This code is already compiled, and it is then linked into the final executable. However, the code is not embedded in the final executable. Instead, the DLL/dylib/so file must be distributed alongside the executable. The executable loads the dynamic library and uses it at runtime. Examples include ffmpeg, fmod, GLEW, libpng, SDL, and zlib.

## Organization
As you might guess, each library has its own subfolder within this `Libraries` folder. But it is the organization of the subfolder itself that is interesting!

Full source libraries (e.g. imgui) are straightforward - the full source code for the library is in the subfolder.

For static and dynamic libraries, it is best to organize the subfolder by platform first, and then by include/lib folders. For example:
	- linux
		- include
		- lib
	- mac
		- include
		- lib
	- win
		- include
		- lib

The primary benefit of this approach is that it allows flexibility for the version of the library to vary by platform. For example, we could upgrade the windows version of the library and not necessarily also be forced to upgrade the other platforms.

Compare that to this approach:
	- include
	- lib
		- linux
		- mac
		- win

In this case, if we update the windows library, the shared header files may be different. This could require us to also update the linux/mac libraries.

To be clear, keeping libraries on the same version across platforms is a GOOD thing. So, do it if you can. But there are times when updating all at once is difficult or unnecessary.

## Upgrading
See the `README.md` in each subfolder for detailed instructions on building and upgrading each library.