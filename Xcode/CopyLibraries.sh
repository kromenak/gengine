#!/bin/sh

if [ -d "$BUILD_DIR/Debug" ]; then
cp "$SRCROOT"/../Libraries/GLEW/lib/mac/*.dylib $BUILD_DIR/Debug
cp "$SRCROOT"/../Libraries/SDL/lib/mac/*.dylib $BUILD_DIR/Debug
cp "$SRCROOT"/../Libraries/libmad/lib/mac/*.dylib $BUILD_DIR/Debug
cp "$SRCROOT"/../Libraries/zlib/lib/mac/*.dylib $BUILD_DIR/Debug
fi

if [ -d "$BUILD_DIR/Release" ]; then
cp "$SRCROOT"/../Libraries/GLEW/lib/mac/*.dylib $BUILD_DIR/Release
cp "$SRCROOT"/../Libraries/SDL/lib/mac/*.dylib $BUILD_DIR/Release
cp "$SRCROOT"/../Libraries/libmad/lib/mac/*.dylib $BUILD_DIR/Release
cp "$SRCROOT"/../Libraries/zlib/lib/mac/*.dylib $BUILD_DIR/Release
fi
