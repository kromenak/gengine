#!/bin/sh

# Copy to exe directory by default.
ASSETS_DIR=$TARGET_BUILD_DIR
LIBS_DIR=$TARGET_BUILD_DIR

# If building mac app, directories are a bit different.
if [ $1 = "app" ]; then
	ASSETS_DIR=$TARGET_BUILD_DIR/$FULL_PRODUCT_NAME/Contents/Resources
	LIBS_DIR=$TARGET_BUILD_DIR/$FULL_PRODUCT_NAME/Contents/Libraries
	mkdir -p "$LIBS_DIR"
fi

# Copy assets folder
cp -r ../Assets "$ASSETS_DIR"
cp -r ../Data "$ASSETS_DIR"
cp ../GK3.ini "$ASSETS_DIR"

# Copy libraries
cp    ../Libraries/ffmpeg/lib/mac/libavcodec.dylib "$LIBS_DIR"
cp    ../Libraries/ffmpeg/lib/mac/libavformat.dylib "$LIBS_DIR"
cp    ../Libraries/ffmpeg/lib/mac/libavutil.dylib "$LIBS_DIR"
cp    ../Libraries/ffmpeg/lib/mac/libswresample.dylib "$LIBS_DIR"
cp    ../Libraries/ffmpeg/lib/mac/libswscale.dylib "$LIBS_DIR"
cp    ../Libraries/fmod/lib/mac/libfmod.dylib "$LIBS_DIR"
cp    ../Libraries/GLEW/lib/mac/libGLEW.dylib "$LIBS_DIR"
cp    ../Libraries/zlib/lib/mac/libz.dylib "$LIBS_DIR"
