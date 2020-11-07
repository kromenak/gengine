This "include" directory is not used on Mac b/c a Framework (with includes embedded) is used there.

I put the header files inside an "SDL2" subfolder so that we could use the same #include statement for both Windows and macOS:

#include <SDL2/SDL.h>