#!/bin/sh

if [ -d "$BUILD_DIR/Debug" ]; then
cp "$SRCROOT"/../Libraries/GLEW/lib/mac/*.dylib $BUILD_DIR/Debug
#cp "$SRCROOT"/../Libraries/SDL/lib/mac/*.dylib $BUILD_DIR/Debug
cp -r "$SRCROOT"/../Libraries/SDL/SDL2.framework $BUILD_DIR/Debug/SDL2.framework
cp -r "$SRCROOT"/../Libraries/SDL/SDL2_mixer.framework $BUILD_DIR/Debug/SDL2_mixer.framework
cp "$SRCROOT"/../Libraries/zlib/lib/mac/*.dylib $BUILD_DIR/Debug
cp "$SRCROOT"/../Libraries/fmod/lib/*.dylib $BUILD_DIR/Debug

cp -R "$SRCROOT"/../Assets $BUILD_DIR/Debug
fi

if [ -d "$BUILD_DIR/Release" ]; then
cp "$SRCROOT"/../Libraries/GLEW/lib/mac/*.dylib $BUILD_DIR/Release
#cp "$SRCROOT"/../Libraries/SDL/lib/mac/*.dylib $BUILD_DIR/Release
cp -r "$SRCROOT"/../Libraries/SDL/SDL2.framework $BUILD_DIR/Release/SDL2.framework
cp -r "$SRCROOT"/../Libraries/SDL/SDL2_mixer.framework $BUILD_DIR/Release/SDL2_mixer.framework
cp "$SRCROOT"/../Libraries/zlib/lib/mac/*.dylib $BUILD_DIR/Release
cp "$SRCROOT"/../Libraries/fmod/lib/*.dylib $BUILD_DIR/Release

cp -R "$SRCROOT"/../Assets $BUILD_DIR/Release
fi
