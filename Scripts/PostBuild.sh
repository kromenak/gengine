#!/bin/sh
SOURCE_DIR=$1

# Note that TARGET_BUILD_DIR and FULL_PRODUCT_NAME are variables provided by Xcode.
# Since this script runs as an Xcode build step, those variables are available.

# Copy to exe directory by default.
LIBS_DIR=${TARGET_BUILD_DIR}

# If building mac app, directories are a bit different.
if [ $2 = "app" ]; then
	LIBS_DIR=${TARGET_BUILD_DIR}/${FULL_PRODUCT_NAME}/Contents/Libraries
	mkdir -p "$LIBS_DIR"
fi

# Copy ffmpeg libraries.
cp ${SOURCE_DIR}/Libraries/ffmpeg/lib/mac/libavcodec.dylib "${LIBS_DIR}"
cp ${SOURCE_DIR}/Libraries/ffmpeg/lib/mac/libavformat.dylib "${LIBS_DIR}"
cp ${SOURCE_DIR}/Libraries/ffmpeg/lib/mac/libavutil.dylib "${LIBS_DIR}"
cp ${SOURCE_DIR}/Libraries/ffmpeg/lib/mac/libswresample.dylib "${LIBS_DIR}"
cp ${SOURCE_DIR}/Libraries/ffmpeg/lib/mac/libswscale.dylib "${LIBS_DIR}"

# Copy fmod library.
cp ${SOURCE_DIR}/Libraries/fmod/lib/mac/libfmod.dylib "${LIBS_DIR}"

# Copy GLEW library.
cp ${SOURCE_DIR}/Libraries/GLEW/lib/mac/libGLEW.dylib "${LIBS_DIR}"

# Copy libz library.
cp ${SOURCE_DIR}/Libraries/zlib/lib/mac/libz.dylib "${LIBS_DIR}"
