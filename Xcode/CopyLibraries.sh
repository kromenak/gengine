#!/bin/sh

cp -r "$SRCROOT"/../Assets $TARGET_BUILD_DIR

cp    "$SRCROOT"/../Libraries/ffmpeg/lib/mac/libavcodec.dylib $TARGET_BUILD_DIR
cp    "$SRCROOT"/../Libraries/ffmpeg/lib/mac/libavformat.dylib $TARGET_BUILD_DIR
cp    "$SRCROOT"/../Libraries/ffmpeg/lib/mac/libavutil.dylib $TARGET_BUILD_DIR
cp    "$SRCROOT"/../Libraries/ffmpeg/lib/mac/libswresample.dylib $TARGET_BUILD_DIR
cp    "$SRCROOT"/../Libraries/ffmpeg/lib/mac/libswscale.dylib $TARGET_BUILD_DIR
cp    "$SRCROOT"/../Libraries/fmod/lib/libfmod.dylib $TARGET_BUILD_DIR
cp    "$SRCROOT"/../Libraries/GLEW/lib/mac/libGLEW.dylib $TARGET_BUILD_DIR
cp -r "$SRCROOT"/../Libraries/SDL/SDL2.framework $TARGET_BUILD_DIR
cp    "$SRCROOT"/../Libraries/zlib/lib/mac/libz.dylib $TARGET_BUILD_DIR