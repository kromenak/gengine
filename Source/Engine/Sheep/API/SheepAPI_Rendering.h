//
// Clark Kromenaker
//
// Sheep SysFuncs related to rendering or capturing what's onscreen.
//
#pragma once
#include "SheepSysFunc.h"

shpvoid EnableIncrementalRendering(); // DEV
shpvoid DisableIncrementalRendering(); // DEV

float GetGamma(); // DEV
shpvoid SetGamma(float gamma); // DEV

shpvoid SetRenderFlat(); // DEV
shpvoid SetRenderFull(); // DEV
shpvoid SetRenderShaded(); // DEV
shpvoid SetRenderWireframe(); // DEV

shpvoid SetShadowTypeBlobby(); // DEV
shpvoid SetShadowTypeModel(); // DEV
shpvoid SetShadowTypeNone(); // DEV

shpvoid SetSurfaceHigh(); // DEV
shpvoid SetSurfaceLow(); // DEV
shpvoid SetSurfaceNormal(); // DEV

shpvoid DrawFilledRect(int left, int top, int right, int bottom,
                       int red, int green, int blue); // DEV

shpvoid RefreshScreen(); // DEV

shpvoid Screenshot();
shpvoid ScreenshotX(const std::string& filename); // DEV

// VIDEO
shpvoid PlayFullScreenMovie(const std::string& movieName); // WAIT
shpvoid PlayFullScreenMovieX(const std::string& movieName, int autoclose); // WAIT
shpvoid PlayMovie(const std::string& movieName); // WAIT
